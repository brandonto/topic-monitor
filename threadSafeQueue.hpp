//******************************************************************************
//
// Copyright (c) 2019, Brandon To
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the author nor the names of its contributors may be
//       used to endorse or promote products derived from this software without
//       specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//******************************************************************************
#ifndef _TOPIC_MONITOR_QUEUE_HPP_
#define _TOPIC_MONITOR_QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>

#include "common.hpp"

namespace topicMonitor
{

template <class T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue(void) {}
    ~ThreadSafeQueue(void) {}

    void push(T entry_p)
    {
        std::unique_lock<std::mutex> lock(mutex_m);
        queue_m.push(entry_p);
        lock.unlock();
        cond_m.notify_one();
    }

    T pop(void)
    {
        std::unique_lock<std::mutex> lock(mutex_m);

        while (queue_m.empty())
        {
            cond_m.wait(lock);
        }

        T entry_p = queue_m.front();
        queue_m.pop();
        return entry_p;
    }

private:
    std::queue<T>           queue_m;
    std::mutex              mutex_m;
    std::condition_variable cond_m;
};

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_QUEUE_HPP_ */
