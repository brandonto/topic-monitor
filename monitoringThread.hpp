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
#ifndef _TOPIC_MONITOR_MONITORING_THREAD_HPP_
#define _TOPIC_MONITOR_MONITORING_THREAD_HPP_

#include <lua5.2/lua.hpp>
#include <solclient/solClient.h>
#include <solclient/solClientMsg.h>
#include <string>
#include <unordered_map>

#include "common.hpp"
#include "threadSafeQueue.hpp"

namespace topicMonitor
{

typedef ThreadSafeQueue<WorkEntry*> InputQueue;
typedef std::unordered_map<std::string, std::string> LuaEnvTable;

class MonitoringThread
{
public:
    static MonitoringThread* instance(void)
    {
        if (instance_mps == nullptr)
        {
            instance_mps = new MonitoringThread();
        }

        return instance_mps;
    }
    ~MonitoringThread(void);

    InputQueue* getInputQueue(void) { return &inputQueue_m; }

    returnCode_t start(void);

private:
    MonitoringThread(void);

    void handleWorkTypeMessageReceived(WorkEntryMessageReceived* entry_p);
    void handleWorkTypeSubscribe(WorkEntrySubscribe* entry_p);
    void handleWorkTypeUnsubscribe(WorkEntryUnsubscribe* entry_p);

    static MonitoringThread* instance_mps;
    InputQueue               inputQueue_m;
    lua_State*               luaState_mp;
    LuaEnvTable              envTable_m;
};

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_MONITORING_THREAD_HPP_ */
