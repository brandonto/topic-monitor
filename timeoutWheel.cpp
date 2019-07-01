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
#include "timeoutWheel.hpp"

#include <sstream>

#include "log.hpp"
#include "monitoringThread.hpp"

namespace topicMonitor
{

void
TimeoutWheel::add(std::string topic, uint32_t timeout)
{
    // The timeout argument is the timeout in seconds. Convert these to a pair
    // of minute:seconds.
    //
    uint32_t minutes = timeout / 60;
    uint32_t seconds = timeout % 60;

    // Calculate the index to insert the TimeoutInfo object by adding the
    // seconds to the index, rolling over at 60 in necessary.
    //
    // i.e.
    //
    // timeout value of 6                 timeout value of 23
    // ==================                 ===================
    // timeout in seconds  = 6            timeout in seconds  = 23
    // timeout in minutes  = 0            timeout in minutes  = 0
    // current wheel index = 40           current wheel index = 58
    // index to insert     = 46           index to insert     = 21
    //
    uint32_t curWheelIndex = ticks_m % 60;
    uint32_t indexToInsert = (curWheelIndex + seconds) % 60;

    // The timeout wheel has a size of 60; tracking every second in a minute, if
    // the timeout is longer than 60 seconds, then we keep an iterationsLeft
    // variable to track the number of times the timeout wheel has to pass this
    // object before it expires. In most cases, this variable is set directly to
    // the number of minutes calculated above. However, due an implementation
    // detail, a special case is made for when the timeout is exactly on a
    // minute boundary (seconds is 0); we have to subtract 1 from the number of
    // minutes.
    //
    // i.e.
    //
    // timeout value of 180               timeout value of 181
    // ====================               ====================
    // timeout in seconds  = 0            timeout in seconds  = 1
    // timeout in minutes  = 3            timeout in minutes  = 3
    // current wheel index = 40           current wheel index = 40
    // index to insert     = 40           index to insert     = 41
    // iterations left     = 2            iterations left     = 3
    //
    uint32_t iterationsLeft = (seconds == 0)?(minutes - 1):minutes;

    TimeoutInfoList& list = wheel_m[indexToInsert];
    list.emplace_back(topic, timeout, iterationsLeft);
}

void
TimeoutWheel::tick(void)
{
    // Iterate through the list of TimeoutInfo objects and checks if they have
    // expired. If they have, delete the object and notify MonitoringThread.
    //
    TimeoutInfoList& list = wheel_m[++ticks_m % 60];
    auto it = list.begin();
    while (it != list.end())
    {
        TimeoutInfo& info = *it;

        // If there are 0 iterations left, then the timeout has expired.
        //
        if (info.getIterationsLeft() == 0)
        {
            // Create a work entry and enqueue it to MonitoringThread's work
            // queue
            //
            WorkEntryTimeout* entry_p = new WorkEntryTimeout();
            entry_p->setTopic(info.getTopic());
            entry_p->setTimeout(info.getTimeout());
            MonitoringThread::instance()->getWorkQueue()->push(entry_p);

            // Delete the object from the list
            //
            it = list.erase(it);
        }
        else
        {
            // Decrement the iterations left
            //
            info.setIterationsLeft(info.getIterationsLeft()-1);
            it++;
        }
    }
}

void
TimeoutWheel::dumpState(void)
{
    std::ostringstream oss;

    for (uint32_t i=0; i<wheel_m.max_size(); i++)
    {
        oss << i << ":{";
        TimeoutInfoList& list = wheel_m[i];
        for (TimeoutInfo& info : list)
        {
            oss << info.getTopic() << ",";
        }
        oss << "}, ";
    }

    LOG(ERROR, oss.str());
}

} /* namespace topicMonitor */
