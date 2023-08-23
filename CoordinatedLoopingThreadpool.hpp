#ifndef COORDINATED_LOOPING_THREADPOOL_HH
#define COORDINATED_LOOPING_THREADPOOL_HH
#include <vector>
#include <thread>
#include <semaphore>



class CoordinatedLoopingThreadpool {
    // Members
    std::vector<std::jthread> workers;               // worker threads
    std::counting_semaphore<> start_sem, finish_sem; // coordination semaphores
    bool synced;                                     // allow trigger() and sync() to be called willy-nilly

public:
    // Constructor; F is a function, args is an iterable
    CoordinatedLoopingThreadpool(auto F, auto args): start_sem(0), finish_sem(0), synced{true} {
        for (auto arg: args) {
            workers.emplace_back([this, F, arg](std::stop_token token){
                while (true) {
                    start_sem.acquire();
                    if (token.stop_requested()) return;
                    F(arg);
                    finish_sem.release();
                }
            });
        }
    }

    // No default, copy, or move constructor is provided
    CoordinatedLoopingThreadpool() = delete;
    CoordinatedLoopingThreadpool(const CoordinatedLoopingThreadpool &) = delete;
    CoordinatedLoopingThreadpool(CoordinatedLoopingThreadpool &&) = delete;

    // Destructor joins workers
    ~CoordinatedLoopingThreadpool() {
        sync();
        for (auto &w: workers) w.request_stop();
        trigger();
    }

    // Block until all threads are finished executing for this iteration
    void sync() {
        if (synced) return;
        for (const auto &w: workers) finish_sem.acquire();
        synced = true;
    }

    // Launch an iteration asynchronously
    void trigger() {
        if (!synced) sync();
        start_sem.release(workers.size());
        synced = false;
    }

    // Launch an iteration and wait for it to complete
    void trigger_sync() {
        trigger();
        sync();
    }
};



#endif
