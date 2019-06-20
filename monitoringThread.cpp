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

#include "solClientThread.hpp"
#include "utils.hpp"

namespace topicMonitor
{

MonitoringThread* MonitoringThread::instance_mps = nullptr;

MonitoringThread::MonitoringThread(void)
{
    luaState_mp = luaL_newstate();
    if (luaState_mp == nullptr) { printf("Fatal error\n"); exit(-1); }

    // Makes all libraries available to lua.
    //
    // TODO (BTO): May want to look into only making a subset of libraries
    //             available to lua.
    //
    luaL_openlibs(luaState_mp);
}

MonitoringThread::~MonitoringThread(void)
{
    lua_close(luaState_mp);
}

void
MonitoringThread::handleWorkTypeMessageReceived(
    WorkEntryMessageReceived* entry_p)
{
    solClient_returnCode_t rc;
    solClient_opaqueMsg_pt msg_p = entry_p->getMsg();

    solClient_destination_t dest;
    rc = solClient_msg_getDestination(msg_p, &dest, sizeof(dest));
    if (rc != SOLCLIENT_OK)
    {
        printf("Error: could not get message topic\n");
        return;
    }

    const char* topic_p = dest.dest;
    if (envTable_m.find(topic_p) == envTable_m.end())
    {
        printf("Error: topic not found in table\n");
        return;
    }
    const char* env_p = envTable_m[topic_p].c_str();

    const char* data_p;
    rc = solClient_msg_getBinaryAttachmentString(msg_p, &data_p);
    if (rc != SOLCLIENT_OK)
    {
        printf("Error: could not get message data\n");
        return;
    }

    if (utils::lua::callMessageFunc(luaState_mp, env_p, data_p)
            != returnCode_t::SUCCESS)
    {
        const char* errorMsg_p = lua_tostring(luaState_mp, -1);
        printf("Error: %s() failed with error \"%s\"\n",
                LUA_MESSAGE_FUNC, errorMsg_p);
        return;
    }
}

void
MonitoringThread::handleWorkTypeSubscribe(WorkEntrySubscribe* entry_p)
{
    returnCode_t rc;

    SubscriptionInfo& info = entry_p->getSubscriptionInfo();

    // Loads lua file into lua state
    //
    rc = utils::lua::loadFileInEnv(luaState_mp,
                                   info.getFilename(),
                                   info.getFilename());
    if (rc == returnCode_t::FAILURE)
    {
        printf("Error: could not load lua file in env for topic %s\n",
                info.getTopic().c_str());
        goto unsubscribe;
    }

    // Check for existence of message function
    //
    if (!utils::lua::isFuncInEnv(luaState_mp,
                                 info.getFilename(),
                                 LUA_MESSAGE_FUNC))
    {
        printf("Error: no %s() found in %s\n",
                LUA_MESSAGE_FUNC, info.getFilename().c_str());
        goto unsubscribe;
    }

    // Check for existence of timer function
    //
    if (info.getTimeout() && !utils::lua::isFuncInEnv(luaState_mp,
                                                      info.getFilename(),
                                                      LUA_TIMER_FUNC))
    {
        printf("Error: no %s() found in %s\n",
                LUA_TIMER_FUNC, info.getFilename().c_str());
        goto unsubscribe;
    }

    // Update table with subscription if everything goes well
    //
    envTable_m[info.getTopic()] = info.getFilename();
    return;

unsubscribe:
    // Unsubscribe from topic
    //
    rc = SolClientThread::instance()->topicUnsubscribe(info.getTopic().c_str());
    if (rc != returnCode_t::SUCCESS)
    {
        printf("Error: could not unsubscribe from topic %s\n",
                info.getTopic().c_str());
        exit(-1);
    }
    return;
}

void
MonitoringThread::handleWorkTypeUnsubscribe(WorkEntryUnsubscribe* entry_p)
{
    printf("handleWorkTypeUnsubscribe()\n\n");
}

// TODO (BTO): Consider using a worker thread pool to dispatch MESSAGE_RECEIVED
//             work items, taking special care to not schedule messages on the
//             same topic to two different worker threads at the same time and
//             preserve ordering.
//
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
