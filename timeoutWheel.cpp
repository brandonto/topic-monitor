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

// TODO (BTO): Provide documentation for this method
//
void
TimeoutWheel::add(std::string topic, uint32_t timeout)
{
    uint32_t minutes = timeout / 60;
    uint32_t seconds = timeout % 60;

    uint32_t curWheelIndex = ticks_m % 60;
    uint32_t indexToInsert = (curWheelIndex + seconds) % 60;

    uint32_t iterationsLeft = (seconds == 0)?(minutes -1):minutes;

    TimeoutInfoList& list = wheel_m[indexToInsert];
    list.emplace_back(topic, timeout, iterationsLeft);
}

// TODO (BTO): Provide documentation for this method
//
void
TimeoutWheel::tick(void)
{
    ticks_m = (ticks_m + 1) % 60;

    TimeoutInfoList& list = wheel_m[ticks_m];
    auto it = list.begin();
    while (it != list.end())
    {
        TimeoutInfo& info = *it;
        if (info.getIterationsLeft() == 0)
        {
            // Create a work entry and enqueue it to MonitoringThread's work
            // queue
            //
            WorkEntryTimeout* entry_p = new WorkEntryTimeout();
            entry_p->setTopic(info.getTopic());
            entry_p->setTimeout(info.getTimeout());
            MonitoringThread::instance()->getWorkQueue()->push(entry_p);

            it = list.erase(it);
        }
        else
        {
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
