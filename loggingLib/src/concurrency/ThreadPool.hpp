#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool{

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex taskMutex;
    std::condition_variable cv;
    bool shutdown = false;

public:
    ThreadPool() = delete;
    ThreadPool(std::size_t numThreads){
        for(std::size_t i = 0 ; i < numThreads ; i++){
            workers.emplace_back([this]() {workerLoop();});  // capture this to access object scope workerLoop function
        }
    }

    ~ThreadPool (){
        {
        // prevent "Lost Wakeup bug".
        // 1. Worker checks 'shutdown' (sees false).
        // 2. Worker gets paused by OS just before calling cv.wait().
        // 3. destructor set 'shutdown=true' and notify_all().
        // 4. Worker resumes and goes to sleep and won't wake (it missed the notify).
        // Every variable used in the same cv.wait() predicate must be modified under the same mutex.
            std::unique_lock<std::mutex> lock(taskMutex);
            shutdown = true;
        } 
        cv.notify_all();  // wake all threads to exit
        for(auto& worker: workers){
            worker.join();
        }
    }
    // Non-copyable, non-movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    bool enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(taskMutex);
            if (shutdown) {
                return false;
            }
            tasks.emplace(std::move(task));
        }
        cv.notify_one();
        return true;
    }

private:
    void workerLoop(){
        while(true){
            std::unique_lock<std::mutex> lock(taskMutex);
            cv.wait(lock , [this](){return !tasks.empty() || shutdown;});

            if (shutdown && tasks.empty()) {   // Exit if signaled to shutdown and queue is empty
                return;
            }

            auto task = std::move(tasks.front());
            tasks.pop();        
            lock.unlock();      // finished operations on shared queue allow other thread to join
            if (task) {
                task();         // start executing the function
            }
        }
    }
};