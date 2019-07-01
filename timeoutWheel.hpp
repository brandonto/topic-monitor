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
#ifndef _TOPIC_MONITOR_TIMEOUT_WHEEL_HPP_
#define _TOPIC_MONITOR_TIMEOUT_WHEEL_HPP_

#include <array>
#include <list>

#include "common.hpp"

namespace topicMonitor
{

class TimeoutInfo
{
public:
    TimeoutInfo(std::string topic, uint32_t timeout, uint32_t iterationsLeft) :
        topic_m(topic),
        timeout_m(timeout),
        iterationsLeft_m(iterationsLeft) {};
    ~TimeoutInfo(void) {}

    void setTopic(std::string topic) { topic_m = topic; }
    std::string getTopic(void) const { return topic_m; }

    void setTimeout(uint32_t timeout) { timeout_m = timeout; }
    uint32_t getTimeout(void) const { return timeout_m; }

    void setIterationsLeft(uint32_t iterationsLeft)
        { iterationsLeft_m = iterationsLeft; }
    uint32_t getIterationsLeft(void) const { return iterationsLeft_m; }

private:
    std::string topic_m;
    uint32_t    timeout_m;
    uint32_t    iterationsLeft_m;
};

// This class maintains a circular array of 60 lists of TimeoutInfo objects that
// correspond to every second in a minute.
//
// When this class is told to track a timeout through the add() method, a
// TimeoutInfo object is created and added to the list of TimeoutInfo objects at
// a calculated index relative to the current index.
//
// Every second, MonitoringThread receives a TIMER event and calls the tick()
// method of this class during the handling of the TIMER event. On each tick,
// the current index is incremented and the list of TimeoutInfo objects at that
// index is traversed. Any TimeoutInfo objects on that list that has expired is
// removed and an event is sent to MonitoringThread with information about the
// timeout.
//
class TimeoutWheel
{
public:
    typedef std::list<TimeoutInfo>          TimeoutInfoList;
    typedef std::array<TimeoutInfoList, 60> TimeoutInfoWheel;

    TimeoutWheel(void) : ticks_m(0) {}
    ~TimeoutWheel(void) {}

    void add(std::string topic, uint32_t timeout);
    void tick(void);
    void dumpState(void);

private:
    TimeoutInfoWheel wheel_m;
    uint32_t         ticks_m;
};

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_TIMEOUT_WHEEL_HPP_ */
