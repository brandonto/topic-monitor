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
#include "log.hpp"

#include <assert.h>
#include <ctime>
#include <iomanip>

namespace topicMonitor
{

Logger* Logger::logger_mps = nullptr;

void
Logger::_log(logLevel_t level,
             const char* file_p,
             int line,
             std::string msg)
{
    if (level < logLevel_m) return;

    // Get the local time as a string
    //
    std::time_t result = std::time(nullptr);
    std::tm *localTime = std::localtime(&result);
    char timeStrBuf[64];
    if (!std::strftime(timeStrBuf, sizeof(timeStrBuf), "%c", localTime)) {
        std::cout << "timeStrBuf too small" << std::endl;
        exit(-1);
    }

    std::string logLevelStr = " [" + _logLevelToString(level) + "]";

    // Prints formatted log string to stream
    //
    stream_m << timeStrBuf;
    stream_m << std::right << std::setw(25) << file_p << ":";
    stream_m << std::left << std::setw(4) << line;
    stream_m << std::setw(9) << logLevelStr;
    stream_m << msg << std::endl;

    // Kill program on FATAL
    //
    if (level == logLevel_t::FATAL) exit(-1);
}

} /* namespace topicMonitor */
