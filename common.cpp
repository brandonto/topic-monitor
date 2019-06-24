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
#include "common.hpp"

namespace topicMonitor
{

std::string returnCodeToString(returnCode_t returnCode)
{
    switch (returnCode)
    {
        case returnCode_t::SUCCESS:       return "SUCCESS";
        case returnCode_t::FAILURE:       return "FAILURE";
        case returnCode_t::NOTHING_TO_DO: return "NOTHING_TO_DO";
    }

    // Control flow should never reach here
    //
    return "";
}

std::string workTypeToString(workType_t workType)
{
    switch (workType)
    {
        case workType_t::MESSAGE_RECEIVED: return "MESSAGE_RECEIVED";
        case workType_t::SUBSCRIBE:        return "SUBSCRIBE";
        case workType_t::UNSUBSCRIBE:      return "UNSUBSCRIBE";
    }

    // Control flow should never reach here
    //
    return "";
}

} /* namespace topicMonitor */
