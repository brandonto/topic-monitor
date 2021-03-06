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
#include "solClientThread.hpp"

#include "log.hpp"
#include "monitoringThread.hpp"

namespace topicMonitor
{

SolClientThread* SolClientThread::instance_mps = nullptr;

static solClient_rxMsgCallback_returnCode_t
sessionMessageReceiveCallback(solClient_opaqueSession_pt session_p,
                              solClient_opaqueMsg_pt msg_p,
                              void* user_p)
{
    LOG(DEBUG, "SolClient message received callback invoked");

    // Create a work entry and enqueue it to MonitoringThread's work queue
    //
    WorkEntryMessageReceived* entry_p = new WorkEntryMessageReceived();
    entry_p->setMsg(msg_p);
    MonitoringThread::instance()->getWorkQueue()->push(entry_p);

    // Taking ownership of the message away from the context thread. We are
    // responsible for freeing the message when done processing.
    //
    return SOLCLIENT_CALLBACK_TAKE_MSG;
}

static void
sessionEventCallback(solClient_opaqueSession_pt session_p,
                     solClient_session_eventCallbackInfo_pt eventInfo_p,
                     void* user_p)
{
    LOG(DEBUG, "SolClient event callback invoked");
}

static void
contextTimerCallback(solClient_opaqueContext_pt context_p, void* user_p)
{
    LOG(DEBUG, "SolClient timer callback invoked");

    // Create a work entry and enqueue it to MonitoringThread's work queue
    //
    WorkEntryTimerTick* entry_p = new WorkEntryTimerTick();
    MonitoringThread::instance()->getWorkQueue()->push(entry_p);
}

// See Messaging API Concepts from the Solace Developer Guide:
//
// https://docs.solace.com/Solace-PubSub-Messaging-APIs/Developer-Guide/Core-Messaging-API-Concepts.htm
//
SolClientThread::SolClientThread(void) :
    context_mp(nullptr),
    session_mp(nullptr)
{
    solClient_returnCode_t rc;

    // solClient_initialize() is called here because SolClientThread is only
    // constructed once.
    //
    rc = solClient_initialize(SOLCLIENT_LOG_DEFAULT_FILTER, nullptr);
    if (rc != SOLCLIENT_OK)
        LOG(FATAL, "solClient initialization failed");

    LOG(INFO, "solClient initialized");

    // Contexts
    //
    // The messaging APIs use processing Contexts for organizing communication
    // between an application and a Solace PubSub+ message broker. Contexts act
    // as containers in which Sessions are created and Session-related events
    // can be handled.
    //
    // A Context encapsulates threads that drive network I/O and message
    // delivery notification for the Sessions and Session components associated
    // with that Context. For the Java API, one thread is used for I/O and
    // another for notification. For the Java RTO, C, and .NET APIs, a single
    // thread is used for both I/O and for notification. The life cycle of a
    // Context‑owned thread is bound to the life cycle of the Context. The
    // Javascript and Node.js APIs are single threaded and have a single global
    // context that is not exposed.
    //
    solClient_context_createFuncInfo_t contextFuncInfo =
        SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    rc = solClient_context_create(
            SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
            &context_mp,
            &contextFuncInfo,
            sizeof(contextFuncInfo));
    if (rc != SOLCLIENT_OK)
        LOG(FATAL, "solClient context creation failed");

    LOG(INFO, "solClient context created");
}

SolClientThread::~SolClientThread(void)
{
    solClient_returnCode_t rc;

    if (session_mp != nullptr)
    {
        rc = solClient_session_destroy(&session_mp);
        if (rc != SOLCLIENT_OK)
            LOG(FATAL, "solClient session destruction failed");
    }

    if (context_mp != nullptr)
    {
        rc = solClient_context_destroy(&context_mp);
        if (rc != SOLCLIENT_OK)
            LOG(FATAL, "solClient context destruction failed");
    }

    // solClient_cleanup() is called here because SolClientThread is only
    // destroyed at program termination.
    //
    rc = solClient_cleanup();
    if (rc != SOLCLIENT_OK)
        LOG(FATAL, "solClient cleanup failed");
}

returnCode_t
SolClientThread::createSession(std::string host,
                               std::string vpn,
                               std::string username,
                               std::string password)
{
    solClient_returnCode_t rc;

    // Sessions
    //
    // When a Context is established, one or more Sessions can be created within
    // that Context. A Session creates a single, client connection to a message
    // broker for sending and receiving messages.
    //
    // A Session provides the following primary services:
    //
    // * client connection
    // * update and retrieve Session properties
    // * retrieve Session statistics
    // * add and remove subscriptions
    // * create destinations and endpoints
    // * publish and receive Direct messages
    // * publish Guaranteed messages
    // * make requests/replies (or create Requestors for the Java API)
    // * create Guaranteed message Flows to receive Guaranteed messages
    // * create Browsers (for the Java and .NET APIs only)
    // * create cache sessions
    //
    // When configuring a Session, the following must be provided:
    //
    // * Session properties to define the operating characteristics of the
    //   client connection to the message broker.
    // * A message callback for Direct messages that are received.
    // * An event handling callback for events that occur for the Session
    //   (optional for the Java API).
    //
    solClient_session_createFuncInfo_t sessionFuncInfo =
        SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

    sessionFuncInfo.rxMsgInfo.callback_p = sessionMessageReceiveCallback;
    sessionFuncInfo.rxMsgInfo.user_p = nullptr;
    sessionFuncInfo.eventInfo.callback_p = sessionEventCallback;
    sessionFuncInfo.eventInfo.user_p = nullptr;

    int propIndex = 0;
    const char* sessionProps[20] = {0};
    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
    sessionProps[propIndex++] = host.c_str();

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
    sessionProps[propIndex++] = vpn.c_str();

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = username.c_str();

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sessionProps[propIndex++] = password.c_str();

    rc = solClient_session_create(
            (char **)sessionProps,
            context_mp,
            &session_mp,
            &sessionFuncInfo,
            sizeof(sessionFuncInfo));
    if (rc != SOLCLIENT_OK)
    {
        LOG(ERROR, "solClient session creation failed");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient session created");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::destroySession(void)
{
    if (session_mp == nullptr) { return returnCode_t::NOTHING_TO_DO; }

    if (solClient_session_destroy(&session_mp) != SOLCLIENT_OK)
    {
        LOG(ERROR, "solClient session destruction failed");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient session destroyed");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::connectSession(void)
{
    if (session_mp == nullptr) { return returnCode_t::FAILURE; }

    std::lock_guard<std::mutex> lock(mutex_m);
    if (solClient_session_connect(session_mp) != SOLCLIENT_OK)
    {
        LOG(ERROR, "solClient session connection failed");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient session connected");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::disconnectSession(void)
{
    if (session_mp == nullptr) { return returnCode_t::FAILURE; }

    std::lock_guard<std::mutex> lock(mutex_m);
    if (solClient_session_disconnect(session_mp) != SOLCLIENT_OK)
    {
        LOG(ERROR, "solClient session disconnection failed");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient session disconnected");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::topicSubscribe(std::string topic)
{
    solClient_returnCode_t rc;
    std::lock_guard<std::mutex> lock(mutex_m);

    rc = solClient_session_topicSubscribeExt(
            session_mp,
            SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
            topic.c_str());
    if (rc != SOLCLIENT_OK)
    {
        LOG(WARN, "solClient could not subscribe to topic '" << topic
                  << "'");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient subscribed to topic '" << topic << "'");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::topicUnsubscribe(std::string topic)
{
    solClient_returnCode_t rc;
    std::lock_guard<std::mutex> lock(mutex_m);

    rc = solClient_session_topicUnsubscribeExt(
            session_mp,
            SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM,
            topic.c_str());
    if (rc != SOLCLIENT_OK)
    {
        LOG(WARN, "solClient could not unsubscribe from topic '" << topic
                  << "'");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient unsubscribed from topic '" << topic << "'");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::startTimer(void)
{
    solClient_returnCode_t rc;
    std::lock_guard<std::mutex> lock(mutex_m);

    rc = solClient_context_startTimer(
            context_mp,
            SOLCLIENT_CONTEXT_TIMER_REPEAT,
            1000, // Timer ticks every second
            contextTimerCallback,
            nullptr,
            &timerId_m);
    if (rc != SOLCLIENT_OK)
    {
        LOG(ERROR, "solClient could not start timer");
        return returnCode_t::FAILURE;
    }

    LOG(INFO, "solClient timer started");
    return returnCode_t::SUCCESS;
}

returnCode_t
SolClientThread::stopTimer(void)
{
    std::lock_guard<std::mutex> lock(mutex_m);

    if (timerId_m == SOLCLIENT_CONTEXT_TIMER_ID_INVALID)
    {
        return returnCode_t::NOTHING_TO_DO;
    }

    if (solClient_context_stopTimer(context_mp, &timerId_m) != SOLCLIENT_OK)
    {
        LOG(ERROR, "solClient could not stop timer");
        return returnCode_t::FAILURE;
    }

    return returnCode_t::SUCCESS;
}

} /* namespace topicMonitor */
