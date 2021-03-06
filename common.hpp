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
#ifndef _TOPIC_MONITOR_COMMON_HPP_
#define _TOPIC_MONITOR_COMMON_HPP_

#include <cstdint>
#include <solclient/solClient.h>
#include <solclient/solClientMsg.h>
#include <string>
#include <vector>

#include "threadSafeQueue.hpp"

namespace topicMonitor
{

const size_t MAX_FILENAME_SIZE = 127;
const char* const LUA_MESSAGE_FUNC = "onMessage";
const char* const LUA_TIMER_FUNC   = "onTimer";

typedef enum class returnCode
{
    SUCCESS,
    FAILURE,
    NOTHING_TO_DO,
} returnCode_t;

std::string returnCodeToString(returnCode_t returnCode);

typedef enum class workType
{
    MESSAGE_RECEIVED,
    SUBSCRIBE,
    UNSUBSCRIBE,
    TIMER_TICK,
    TIMEOUT,
} workType_t;

std::string workTypeToString(workType_t workType);

class SubscriptionInfo
{
public:
    SubscriptionInfo(void) : timeout_m(0) {}
    ~SubscriptionInfo(void) {}

    bool setTopic(std::string topic)
    {
        if (topic.length() > SOLCLIENT_BUFINFO_MAX_TOPIC_SIZE) { return false; }
        topic_m = topic;
        return true;
    }
    std::string getTopic(void) const { return topic_m; }

    bool setFilename(std::string filename)
    {
        if (filename.length() > MAX_FILENAME_SIZE) { return false; }
        filename_m = filename;
        return true;
    }
    std::string getFilename(void) const { return filename_m; }

    void setTimeout(uint32_t timeout) { timeout_m = timeout; }
    uint32_t getTimeout(void) const { return timeout_m; }

private:
    std::string topic_m;
    std::string filename_m;
    uint32_t    timeout_m;
};
typedef std::vector<SubscriptionInfo> SubscriptionInfoList;

// TODO (BTO): Consider using a pool allocator to allocate these objects
//
class WorkEntry
{
public:
    WorkEntry(workType_t type) : type_m(type) {}
    virtual ~WorkEntry(void) {}

    void setType(workType_t type) { type_m = type; }
    workType_t getType(void) const { return type_m; }

private:
    workType_t type_m;
};

class WorkEntryMessageReceived : public WorkEntry
{
public:
    WorkEntryMessageReceived(void) : WorkEntry(workType_t::MESSAGE_RECEIVED) {}
    ~WorkEntryMessageReceived(void) { solClient_msg_free(&msg_mp); }

    void setMsg(solClient_opaqueMsg_pt msg_p) { msg_mp = msg_p; }
    solClient_opaqueMsg_pt getMsg(void) const { return msg_mp; }

private:
    solClient_opaqueMsg_pt msg_mp;
};

class WorkEntrySubscribe : public WorkEntry
{
public:
    WorkEntrySubscribe(void) : WorkEntry(workType_t::SUBSCRIBE) {}
    ~WorkEntrySubscribe(void) {}

    void setSubscriptionInfo(SubscriptionInfo info) { info_m = info; }
    const SubscriptionInfo& getSubscriptionInfo(void) const { return info_m; }

private:
    SubscriptionInfo info_m;
};

class WorkEntryUnsubscribe : public WorkEntry
{
public:
    WorkEntryUnsubscribe(void) : WorkEntry(workType_t::UNSUBSCRIBE) {}
    ~WorkEntryUnsubscribe(void) {}

    void setSubscriptionInfo(SubscriptionInfo info) { info_m = info; }
    const SubscriptionInfo& getSubscriptionInfo(void) const { return info_m; }

private:
    SubscriptionInfo info_m;
};

class WorkEntryTimerTick : public WorkEntry
{
public:
    WorkEntryTimerTick(void) : WorkEntry(workType_t::TIMER_TICK) {}
    ~WorkEntryTimerTick(void) {}
};

class WorkEntryTimeout : public WorkEntry
{
public:
    WorkEntryTimeout(void) : WorkEntry(workType_t::TIMEOUT) {}
    ~WorkEntryTimeout(void) {}

    void setTopic(std::string topic) { topic_m = topic; }
    std::string getTopic(void) const { return topic_m; }

    void setTimeout(uint32_t timeout) { timeout_m = timeout; }
    uint32_t getTimeout(void) const { return timeout_m; }

private:
    std::string topic_m;
    uint32_t    timeout_m;
};

typedef ThreadSafeQueue<WorkEntry*> WorkQueue;

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_COMMON_HPP_ */
