// -*- C++ -*-

#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore
{
private:
        int count;
        std::mutex mutex;
        std::condition_variable cond;

public:
        Semaphore(int c = 0) : count(c) {};
        ~Semaphore() {}

        void up()
        {
                // Why is the lock not needed for notify_one() ?
                {
                        std::unique_lock<std::mutex> lock(mutex);
                        ++count;
                }
                cond.notify_one();
        }

        void down()
        {
                std::unique_lock<std::mutex> lock(mutex);
                cond.wait(lock, [this]{ return this->count; });
                --count;
        }
};
