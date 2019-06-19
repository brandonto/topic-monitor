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
#ifndef _TOPIC_MONITOR_SOLCLIENT_THREAD_HPP_
#define _TOPIC_MONITOR_SOLCLIENT_THREAD_HPP_

#include <mutex>
#include <solclient/solClient.h>
#include <solclient/solClientMsg.h>

#include "common.hpp"

namespace topicMonitor
{

class SolClientThread
{
public:
    static SolClientThread* instance(void)
    {
        if (instance_mps == nullptr)
        {
            instance_mps = new SolClientThread();
        }

        return instance_mps;
    }

    ~SolClientThread(void);

    returnCode_t createSession(const char* host_p,
                               const char* vpn_p,
                               const char* username_p,
                               const char* password_p);
    returnCode_t destroySession(void);

    returnCode_t connectSession(void);
    returnCode_t disconnectSession(void);

    returnCode_t topicSubscribe(const char* topic_p);
    returnCode_t topicUnsubscribe(const char* topic_p);

private:
    SolClientThread(void);

    static SolClientThread*    instance_mps;
    solClient_opaqueContext_pt context_mp;
    solClient_opaqueSession_pt session_mp;
    std::mutex                 mutex_m;
};

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_SOLCLIENT_THREAD_HPP_ */
