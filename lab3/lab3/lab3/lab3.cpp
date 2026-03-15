#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <atomic>
#include <random>
#include <iomanip>

using namespace std;

struct Task {
    function<void()> work;
    int duration;
    int id;
};

class ThreadPool {
private:
    vector<thread> workers;
    queue<Task> tasks;
    mutex mtx;
    condition_variable cv;
    int total_queued_time = 0;
    bool stop_graceful = false;
    bool stop_immediate = false;
    bool is_paused = false;
    atomic<int> rejected_tasks{ 0 };
    atomic<int> completed_tasks{ 0 };
    vector<chrono::duration<double>> worker_wait_times;

public:
    ThreadPool(size_t threads_count) {
        worker_wait_times.resize(threads_count, chrono::duration<double>(0));
        for (size_t i = 0; i < threads_count; ++i) {
            workers.emplace_back([this, i] {
                while (true) {
                    Task task;
                    {
                        unique_lock<mutex> lock(mtx);

                        auto wait_start = chrono::steady_clock::now();
                        cv.wait(lock, [this] {
                            return stop_immediate ||
                                (stop_graceful && tasks.empty()) ||
                                (!tasks.empty() && !is_paused);
                            });
                        auto wait_end = chrono::steady_clock::now();
                        worker_wait_times[i] += (wait_end - wait_start);

                        if (stop_immediate || (stop_graceful && tasks.empty())) {
                            return;
                        }

                        task = move(tasks.front());
                        tasks.pop();
                        total_queued_time -= task.duration;
                    }

                    cout << "[Worker " << i << "] Started Task #" << task.id << " (" << task.duration << "s)\n";
                    task.work();
                    completed_tasks++;
                }
            });
        }
    }

    bool enqueue(int id, int duration) {
        lock_guard<mutex> lock(mtx);
        if (total_queued_time + duration > 60) {
            rejected_tasks++;
            return false;
        }
        Task new_task;
        new_task.id = id;
        new_task.duration = duration;
        new_task.work = [duration]() {
            this_thread::sleep_for(chrono::seconds(duration));
            };
        tasks.push(new_task);
        total_queued_time += duration;
        cv.notify_one();
        return true;
    }

    void pause() {
        lock_guard<mutex> lock(mtx);
        is_paused = true;
    }

    void resume() {
        {
            lock_guard<mutex> lock(mtx);
            is_paused = false;
        }
        cv.notify_all();
    }

    void shutdown(bool immediate) {
        {
            lock_guard<mutex> lock(mtx);
            if (immediate) stop_immediate = true;
            else stop_graceful = true;
        }
        cv.notify_all();
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
    }

    void printMetrics() {
        cout << "\n--------------- LAB 3 STATA ----------------\n";
        cout << "Threads in pool: " << workers.size() << "\n";
        cout << "Completed tasks: " << completed_tasks << "\n";
        cout << "Rejected tasks (limit > 60s): " << rejected_tasks << "\n";

        double total_wait = 0;
        for (size_t i = 0; i < worker_wait_times.size(); ++i) {
            cout << "Worker [" << i << "] vacant time: " << fixed << setprecision(2) << worker_wait_times[i].count() << "s\n";
            total_wait += worker_wait_times[i].count();
        }
        cout << "Average vacant time: " << total_wait / workers.size() << "s\n";
        cout << "------------------------------------------------\n";
    }

    ~ThreadPool() {
        shutdown(false);
    }
};

void taskProducer(ThreadPool& pool, int producer_id, atomic<bool>& stop) {
    int counter = 0;

    while (!stop) {
        int duration = rand() % 8 + 5;
        int task_id = producer_id * 100 + counter++;

        if (pool.enqueue(task_id, duration)) {
            cout << "[P" << producer_id << "] ADD #" << task_id << " (" << duration << "s)" << endl;
        }
        else {
            cout << "[P" << producer_id << "] REJECTED #" << task_id << endl;
        }

        this_thread::sleep_for(chrono::seconds(rand() % 3 + 1));
    }
}

int main() {
    srand(time(0));
    ThreadPool pool(4);
    atomic<bool> stop_producing{ false };

    thread p1(taskProducer, ref(pool), 1, ref(stop_producing));
    thread p2(taskProducer, ref(pool), 2, ref(stop_producing));

    this_thread::sleep_for(chrono::seconds(15));
    pool.pause();
    this_thread::sleep_for(chrono::seconds(7));
    pool.resume();
    this_thread::sleep_for(chrono::seconds(15));

    cout << "\nStopping producers...\n";
    stop_producing = true;
    p1.join();
    p2.join();

    cout << "Finalizing remaining tasks...\n";
    pool.shutdown(false);

    pool.printMetrics();

    return 0;
}