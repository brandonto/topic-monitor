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
#include <cxxabi.h>
#include <iomanip>

// We only need local unwinding. Defining UNW_LOCAL_ONLY tells libunwind to
// select a special implementation optimized for local unwinding.
//
#define UNW_LOCAL_ONLY
#include <libunwind.h>

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
    if (!std::strftime(timeStrBuf, sizeof(timeStrBuf), "%c", localTime))
    {
        std::cout << "timeStrBuf too small" << std::endl;
        exit(-1);
    }

    std::string logLevelStr = " [" + logLevelToString(level) + "]";

    // Outputs formatted log string to stream
    //
    stream_m << timeStrBuf;
    stream_m << std::right << std::setw(25) << file_p << ":";
    stream_m << std::left << std::setw(4) << line;
    stream_m << std::setw(9) << logLevelStr;
    stream_m << msg << std::endl;
}

static void
_libunwindErrorHandler(std::ostream& s, std::string funcName, unw_error_t err)
{
    // TODO (BTO): Just default to error output and exiting in the mean time...
    //             decide how to handle libunwind failures later
    //
    s << funcName << "() failed with ";

    switch (err)
    {
    case UNW_EUNSPEC:
        s << "UNW_EUNSPEC: \"unspecified (general) error\"";
        break;
    case UNW_ENOMEM:
        s << "UNW_ENOMEM: \"out of memory\"";
        break;
    case UNW_EBADREG:
        s << "UNW_EBADREG: \"bad register number\"";
        break;
    case UNW_EREADONLYREG:
        s << "UNW_EREADONLYREG: \"attempt to write read-only register\"";
        break;
    case UNW_ESTOPUNWIND:
        s << "UNW_ESTOPUNWIND: \"stop unwinding\"";
        break;
    case UNW_EINVALIDIP:
        s << "UNW_EINVALIDIP: \"invalid IP\"";
        break;
    case UNW_EBADFRAME:
        s << "UNW_EBADFRAME: \"bad frame\"";
        break;
    case UNW_EINVAL:
        s << "UNW_EINVAL: \"unsupported operation or bad value\"";
        break;
    case UNW_EBADVERSION:
        s << "UNW_EBADVERSION: \"unwind info has unsupported version\"";
        break;
    case UNW_ENOINFO:
        s << "UNW_ENOINFO: \"no unwind info found\"";
        break;
    default:
        s << "unknown error code";
        break;
    }

    s << std::endl;
    exit(-1);
}

void
Logger::_backtrace(logLevel_t level,
                   const char* file_p,
                   int line)
{
    if (level < logLevel_m) return;

    // Get the local time as a string
    //
    std::time_t result = std::time(nullptr);
    std::tm *localTime = std::localtime(&result);
    char timeStrBuf[64];
    if (!std::strftime(timeStrBuf, sizeof(timeStrBuf), "%c", localTime))
    {
        std::cout << "timeStrBuf too small" << std::endl;
        exit(-1);
    }

    std::string logLevelStr = " [" + logLevelToString(level) + "]";

    // Get a snapshot of the CPU registers (machine-state)
    //
    unw_context_t context;
    int ret;
    if ((ret = unw_getcontext(&context)) != UNW_ESUCCESS)
    {
        _libunwindErrorHandler(stream_m, "unw_getcontext", (unw_error_t)(-ret));
    }

    // Initialize unwind cursor using context. The cursor should now point to
    // the current frame in the call stack
    //
    unw_cursor_t cursor;
    if ((ret = unw_init_local(&cursor, &context)) != UNW_ESUCCESS)
    {
        _libunwindErrorHandler(stream_m, "unw_init_local", (unw_error_t)(-ret));
    }

    // Iterate through all frames on the call stack and output the demangled
    // procedure names along with other relevent information
    //
    unw_word_t ip, offset;
    char sym[256];
    while ((ret = unw_step(&cursor)) != UNW_ESUCCESS)
    {
        if (ret < UNW_ESUCCESS)
        {
            _libunwindErrorHandler(stream_m, "unw_step", (unw_error_t)(-ret));
        }

        // Read the instruction pointer register
        //
        if ((ret = unw_get_reg(&cursor, UNW_REG_IP, &ip)) != UNW_ESUCCESS)
        {
            _libunwindErrorHandler(stream_m, "unw_get_reg", (unw_error_t)(-ret));
        }

        // Return the name of the procedure
        //
        if ((ret = unw_get_proc_name(&cursor, sym, sizeof(sym), &offset))
            != UNW_ESUCCESS)
        {
            _libunwindErrorHandler(stream_m, "unw_get_proc_name",
                (unw_error_t)(-ret));
        }

        // Demangle the procedure name
        //
        int status;
        char* procedureName_p = sym;
        char* demangledName_p = abi::__cxa_demangle(sym,
                                                    nullptr,
                                                    nullptr,
                                                    &status);
        if (status == 0) { procedureName_p = demangledName_p; }

        // Outputs formatted stack frame to stream
        //
        stream_m << timeStrBuf;
        stream_m << std::right << std::setw(25) << file_p << ":";
        stream_m << std::left << std::setw(4) << line;
        stream_m << std::setw(9) << logLevelStr;
        stream_m << "0x" << std::hex << ip;
        stream_m << ": (" << procedureName_p;
        stream_m << "+0x" << std::hex << offset << ")" << std::endl;
    }
}

} /* namespace topicMonitor */
