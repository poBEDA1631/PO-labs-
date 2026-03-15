#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <atomic>

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

public:
    ThreadPool(size_t threads_count) {
        for (size_t i = 0; i < threads_count; ++i) {
            workers.emplace_back([this, i] {
                while (true) {
                    Task task;
                    {
                        unique_lock<mutex> lock(mtx);

                        cv.wait(lock, [this] {
                            return stop_immediate ||
                                (stop_graceful && tasks.empty()) ||
                                (!tasks.empty() && !is_paused);
                            });

                        if (stop_immediate || (stop_graceful && tasks.empty())) {
                            return;
                        }

                        task = move(tasks.front());
                        tasks.pop();
                        total_queued_time -= task.duration;
                    }

                    cout << "[Thread " << i << "] Processing task #" << task.id << endl;
                    task.work();
                }
                });
        }
    }

    bool enqueue(int id, int duration) {
        lock_guard<mutex> lock(mtx);
        if (total_queued_time + duration > 60) {
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

    ~ThreadPool() {
        shutdown(false);
    }
};

int main() {
    ThreadPool pool(4);

    for (int i = 1; i <= 5; ++i) {
        if (pool.enqueue(i, 5)) {
            cout << "Task " << i << " enqueued." << endl;
        }
    }

    this_thread::sleep_for(chrono::seconds(2));
    pool.pause();
    cout << "Pool paused... please wait and don't get bored" << endl;

    this_thread::sleep_for(chrono::seconds(3));
    pool.resume();

    pool.shutdown(false);
    cout << "Pool shut down." << endl;

    return 0;
}