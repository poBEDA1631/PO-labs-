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
protected:
    vector<thread> workers;
    queue<Task> tasks;
    mutex mtx;
    condition_variable cv;
    int total_queued_time = 0;

public:
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
};