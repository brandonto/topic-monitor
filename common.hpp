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

#include <cstring>
#include <cstdint>
#include <solclient/solClient.h>
#include <solclient/solClientMsg.h>
#include <vector>

namespace topicMonitor
{

typedef enum class returnCode
{
    SUCCESS,
    FAILURE,
    NOTHING_TO_DO,
} returnCode_t;

typedef enum class workType
{
    MESSAGE_RECEIVED,
    SUBSCRIBE,
    UNSUBSCRIBE,
} workType_t;

// TODO (BTO): Create a move constructor for this object because it is currently
//             being copied into WorkEntrySubscribe.
//
class SubscriptionInfo
{
public:
    static const size_t MAX_FILENAME_SIZE = 127;
    SubscriptionInfo(void) {}
    ~SubscriptionInfo(void) {}

    bool setTopic(const char* topic_p, size_t len)
    {
        if (len > SOLCLIENT_BUFINFO_MAX_TOPIC_SIZE) { return false; }
        strncpy(topic_ma, topic_p, len);
        topic_ma[len] = '\0';
        return true;
    }
    const char* getTopic(void) const { return topic_ma; }

    bool setFilename(const char* filename_p, size_t len)
    {
        if (len > MAX_FILENAME_SIZE) { return false; }
        strncpy(filename_ma, filename_p, len);
        filename_ma[len] = '\0';
        return true;
    }
    const char* getFilename(void) const { return filename_ma; }

    void setTimeout(uint32_t timeout) { timeout_m = timeout; }
    uint32_t getTimeout(void) const { return timeout_m; }

private:
    char     topic_ma[SOLCLIENT_BUFINFO_MAX_TOPIC_SIZE + 1];
    char     filename_ma[MAX_FILENAME_SIZE + 1];
    uint32_t timeout_m;
};
typedef std::vector<SubscriptionInfo> SubscriptionInfoList;

// Consider using a custom pool allocator to allocate these objects.
class WorkEntry
{
public:
    WorkEntry(workType_t type) : type_m(type) {}
    virtual ~WorkEntry(void) {}

    void setType(workType_t type) { type_m = type; }
    workType_t getType(void) const { return type_m; }

private:
    workType_t             type_m;
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

    bool setSubscriptionInfo(SubscriptionInfo info) { info_m = info; }
    SubscriptionInfo& getSubscriptionInfo(void) { return info_m; }

private:
    SubscriptionInfo info_m;
};

class WorkEntryUnsubscribe : public WorkEntry
{
public:
    WorkEntryUnsubscribe(void) : WorkEntry(workType_t::UNSUBSCRIBE) {}
    ~WorkEntryUnsubscribe(void) {}

    bool setSubscriptionInfo(SubscriptionInfo info) { info_m = info; }
    SubscriptionInfo& getSubscriptionInfo(void) { return info_m; }

private:
    SubscriptionInfo info_m;
};

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_COMMON_HPP_ */
