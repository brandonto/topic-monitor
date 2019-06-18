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
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <lua5.2/lua.hpp>

#include "common.hpp"
#include "monitoringThread.hpp"
#include "solClientThread.hpp"
#include "threadSafeQueue.hpp"
#include "utils.hpp"

namespace topicMonitor
{

// TODO (BTO): Maybe use a smart pointer with a Deleter FunctionObject here to
//             clean up the lua_State once it goes out of scope?
returnCode_t
createAndStartSolClientThread()
{
    returnCode_t rc;

    // Create and initialize SolClientThread
    //
    SolClientThread* thread_p = SolClientThread::instance();

    const char* host_p;
    const char* vpn_p;
    const char* username_p;
    const char* password_p;

    // Opens a new lua state and load credentials.lua
    //
    lua_State* L = luaL_newstate();
    luaopen_base(L);
    if (luaL_dofile(L, "credentials.lua") != 0)
    {
        printf("Error: cannot load credentials.lua\n");
        goto cleanup;
    }

    // Extract credentials from credentials.lua
    //
    // TODO (BTO): Proper error output if file is malformed
    //
    host_p = luaUtils::getStringValueFromSymbol(L, "host");
    if (host_p == nullptr) { goto cleanup; }
    vpn_p = luaUtils::getStringValueFromSymbol(L, "vpn");
    if (vpn_p == nullptr) { goto cleanup; }
    username_p = luaUtils::getStringValueFromSymbol(L, "username");
    if (username_p == nullptr) { goto cleanup; }
    password_p = luaUtils::getStringValueFromSymbol(L, "password");
    if (password_p == nullptr) { goto cleanup; }

    // Create session
    //
    rc = thread_p->createSession(host_p, vpn_p, username_p, password_p);
    if (rc != returnCode_t::SUCCESS) { goto cleanup; }

    // Connect to message broker
    //
    rc = thread_p->connectSession();
    if (rc != returnCode_t::SUCCESS) { goto cleanup; }

    lua_close(L);
    return returnCode_t::SUCCESS;

cleanup:
    lua_close(L);
    return returnCode_t::FAILURE;
}

// TODO (BTO): Maybe use a smart pointer with a Deleter FunctionObject here to
//             clean up the lua_State once it goes out of scope?
returnCode_t
getSubscriptionInfoList(SubscriptionInfoList& subscriptions)
{
    // Opens a new lua state and load subscriptionTable.lua
    //
    lua_State* L = luaL_newstate();
    luaopen_base(L);
    if (luaL_dofile(L, "subscriptionTable.lua") != 0)
    {
        printf("Error: cannot load subscriptionTable.lua\n");
        goto cleanup;
    }

    // Push global table subscriptionTable from lua file onto the stack
    //
    lua_getglobal(L, "subscriptionTable");
    if (!lua_istable(L, -1))
    {
        printf("Error: subscriptionTable invalid format\n");
        goto cleanup;
    }

    // Iterate through all elements of subscriptionTable and populate the
    // subscription list.
    //
    // A table entry has the following format:
    //
    // key: <topic:string>
    // value: table { 
    //          key: "filename", value: <filename:string>,
    //          key: "timer", value: <seconds:int>,        (optional)
    //        }
    //
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // subscriptionTable should contain (string, table) key-value pairs
        //
        if (!lua_isstring(L, -2) || !lua_istable(L, -1))
        {
            printf("Error: subscriptionTable invalid format\n");
            goto cleanup;
        }

        const char* topic_p = lua_tostring(L, -2);

        SubscriptionInfo info;
        info.setTopic(topic_p, strlen(topic_p));

        // Parse value field of subscriptionTable (which is another table)
        //
        bool seenFile = false;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            // All keys in this table should be strings
            //
            if (!lua_isstring(L, -2))
            {
                printf("Error: subscriptionTable invalid format (key not string)\n");
                goto cleanup;
            }

            const char* key_p = lua_tostring(L, -2);
            if (strcmp(key_p, "filename") == 0)
            {
                seenFile = true;
                if (!lua_isstring(L, -1))
                {
                    printf("Error: subscriptionTable invalid format (filename value not string)\n");
                    goto cleanup;
                }
                const char* filename_p = lua_tostring(L, -1);
                info.setFilename(filename_p, strlen(filename_p));
            }
            else if (strcmp(key_p, "timer") == 0)
            {
                // TODO (BTO): This should check for integers, not numbers...
                //             fix later.
                if (!lua_isnumber(L, -1))
                {
                    printf("Error: subscriptionTable invalid format (timer value not integer)\n");
                    goto cleanup;
                }
                uint32_t timeout = lua_tonumber(L, -1);
                info.setTimeout(timeout);
            }
            else
            {
                printf("Error: subscriptionTable invalid format (unknown key)\n");
                goto cleanup;
            }

            // Remove 'value'... keep 'key' for next iteration
            //
            lua_pop(L, 1);
        }

        if (!seenFile)
        {
            printf("Error: subscriptionTable invalid format (filename not seen)\n");
            goto cleanup;
        }

        subscriptions.push_back(info);

        // Remove 'value'... keep 'key' for next iteration
        //
        lua_pop(L, 1);
    }

    lua_close(L);
    return returnCode_t::SUCCESS;

cleanup:
    lua_close(L);
    return returnCode_t::FAILURE;
}

returnCode_t
subscribeToMonitoredTopics(void)
{
    returnCode_t rc;

    SolClientThread* thread_p = SolClientThread::instance();

    // TODO (BTO): Parse through files to find subscription list and subscribe
    //             to each one.
    //
    SubscriptionInfoList subscriptions;
    rc = getSubscriptionInfoList(subscriptions);
    if (rc != returnCode_t::SUCCESS) { return returnCode_t::FAILURE; }

    for (auto it = subscriptions.begin(); it < subscriptions.end(); it++)
    {
        //printf("%s, %s, %u\n", it->getTopic(), it->getFilename(), it->getTimeout());
        rc = thread_p->topicSubscribe(it->getTopic());
        if (rc != returnCode_t::SUCCESS) { return returnCode_t::FAILURE; }

        // Create a work entry and enqueue it to MonitoringThread's input queue
        //
        WorkEntrySubscribe* entry_p = new WorkEntrySubscribe();
        entry_p->setSubscriptionInfo(*it);
        MonitoringThread::instance()->getInputQueue()->push(entry_p);
    }

    return returnCode_t::SUCCESS;
}

returnCode_t
unsubscribeFromMonitoredTopics(void)
{
    returnCode_t rc;

    SolClientThread* thread_p = SolClientThread::instance();

    // TODO (BTO): Unsubscribe to each topic.
    //
    rc = thread_p->topicUnsubscribe("temperature");
    if (rc != returnCode_t::SUCCESS) { return returnCode_t::FAILURE; }

    return returnCode_t::SUCCESS;
}

returnCode_t
createAndStartMonitoringThread(void)
{
    returnCode_t rc;

    // Create and initialize MonitoringThread
    //
    MonitoringThread* thread_p = MonitoringThread::instance();

    // Subscribe to all monitored topics. This must be called after
    // MonitoringThread is created because it will push work entries on the
    // input queue of MonitoringThread.
    //
    if (subscribeToMonitoredTopics() != returnCode_t::SUCCESS)
    {
        printf("Error subscribing to monitored topics.\n");
        return returnCode_t::FAILURE;
    }

    // Do main loop
    //
    rc = thread_p->start();
    if (rc != returnCode_t::SUCCESS) { return returnCode_t::FAILURE; }

    // Unsubscribe from all monitored topics.
    //
    if (unsubscribeFromMonitoredTopics() != returnCode_t::SUCCESS)
    {
        printf("Error subscribing to monitored topics.\n");
        return returnCode_t::FAILURE;
    }

    return returnCode_t::SUCCESS;
}

} /* namespace topicMonitor */

int
main(int argc, char* argv[])
{
    using namespace topicMonitor;

    if (createAndStartSolClientThread() != returnCode_t::SUCCESS)
    {
        printf("Error creating and starting SolClientThread.\n");
        return -1;
    }

    if (createAndStartMonitoringThread() != returnCode_t::SUCCESS)
    {
        printf("Error creating and starting MonitoringThread.\n");
        return -1;
    }

    return 0;
}
