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
#include "monitoringThread.hpp"

namespace topicMonitor
{

MonitoringThread* MonitoringThread::instance_mps = nullptr;

MonitoringThread::MonitoringThread(void)
{
    luaState_mp = luaL_newstate();
    if (luaState_mp == nullptr) { printf("Fatal error\n"); exit(-1); }
}

MonitoringThread::~MonitoringThread(void)
{
    lua_close(luaState_mp);
}

void
MonitoringThread::handleWorkTypeMessageReceived(
    WorkEntryMessageReceived* entry_p)
{
    solClient_opaqueMsg_pt msg_p = entry_p->getMsg();

    uint8_t* smf_p;
    uint32_t smfLen;
    solClient_returnCode_t rc;
    rc = solClient_msg_getSMFPtr(msg_p, &smf_p, &smfLen);
    if (rc != SOLCLIENT_OK)
    {
        printf("Fatal error\n"); exit(-1);
    }

    printf("handleWorkTypeMessageReceived(): Received message:\n");
    solClient_msg_dump(msg_p, nullptr, 0);
    printf("\n");
}

void
MonitoringThread::handleWorkTypeSubscribe(WorkEntrySubscribe* entry_p)
{
    SubscriptionInfo& info = entry_p->getSubscriptionInfo();
    printf("handleWorkTypeSubscribe()\n");
    printf("%s, %s, %u\n\n", info.getTopic(), info.getFilename(), info.getTimeout());
}

void
MonitoringThread::handleWorkTypeUnsubscribe(WorkEntryUnsubscribe* entry_p)
{
    printf("handleWorkTypeUnsubscribe()\n\n");
}

returnCode_t
MonitoringThread::start(void)
{
    WorkEntry* entry_p = nullptr;

    for (;;entry_p = nullptr)
    {
        entry_p = inputQueue_m.pop();
        if (entry_p == nullptr) { printf("Fatal error\n"); exit(-1); }

        switch (entry_p->getType())
        {
        case workType_t::MESSAGE_RECEIVED:
            handleWorkTypeMessageReceived(
                static_cast<WorkEntryMessageReceived*>(entry_p));
            break;
        case workType_t::SUBSCRIBE:
            handleWorkTypeSubscribe(
                static_cast<WorkEntrySubscribe*>(entry_p));
            break;
        case workType_t::UNSUBSCRIBE:
            handleWorkTypeUnsubscribe(
                static_cast<WorkEntryUnsubscribe*>(entry_p));
            break;
        default:
            printf("Unknown work type.\n");
            return returnCode_t::FAILURE;
        }

        delete entry_p;
    } while (1);

    return returnCode_t::SUCCESS;
}

} /* namespace topicMonitor */
