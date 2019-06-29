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
#ifndef _TOPIC_MONITOR_LOG_HPP_
#define _TOPIC_MONITOR_LOG_HPP_

#include <iostream>
#include <sstream>
#include <string>

#define LOG_STREAM(logLevel, msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    topicMonitor::Logger::log(Logger::logLevel_t::logLevel, __FILENAME__, __LINE__, oss.str()); \
} while (0)

// This macro is the primary interface to the logging system
//
#define LOG(logLevel, msg) do { \
    LOG_STREAM(logLevel, msg); \
    if (Logger::logLevel_t::logLevel == Logger::logLevel_t::FATAL) exit(-1); \
} while (0)

// This macro will include a backtrace in the logs, useful for debugging
//
#define LOG_TRACE(logLevel, msg) do { \
    LOG_STREAM(logLevel, msg); \
    topicMonitor::Logger::backtrace(Logger::logLevel_t::logLevel, __FILENAME__, __LINE__); \
    if (Logger::logLevel_t::logLevel == Logger::logLevel_t::FATAL) exit(-1); \
} while (0)

namespace topicMonitor
{

class Logger
{
public:
    typedef enum class logLevel
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    } logLevel_t;

    // Should be called on startup before any logging is done.
    //
    static void init(std::ostream& s, logLevel_t level = logLevel_t::WARN)
    {
        if (logger_mps == nullptr) {
            logger_mps = new Logger(s, level);
            LOG(INFO, "Logger initialized.");
        }
        else {
            LOG(WARN, "Logger has already been initialized.");
        }
    }

    static void log(logLevel_t  level,
                    const char* file_p,
                    int         line,
                    std::string msg)
    {
        Logger::getInstance()->_log(level, file_p, line, msg);
    }

    static void backtrace(logLevel_t  level,
                          const char* file_p,
                          int         line)
    {
        Logger::getInstance()->_backtrace(level, file_p, line);
    }

private:
    Logger(std::ostream &s, logLevel_t level) :
        stream_m(s),
        logLevel_m(level)
    {
    }

    static Logger* getInstance()
    {
        return logger_mps;
    }

    void _log(logLevel_t  level,
              const char* file_p,
              int         line,
              std::string msg);

    void _backtrace(logLevel_t  level,
                    const char* file_p,
                    int         line);

    std::string logLevelToString(logLevel_t level)
    {
        switch (level)
        {
            case logLevel_t::DEBUG: return "DEBUG";
            case logLevel_t::INFO:  return "INFO";
            case logLevel_t::WARN:  return "WARN";
            case logLevel_t::ERROR: return "ERROR";
            case logLevel_t::FATAL: return "FATAL";
        }

        // Control flow should never reach here
        //
        return "";
    }

    static Logger* logger_mps;
    std::ostream&  stream_m;
    logLevel_t     logLevel_m;
};

} /* namespace topicMonitor */

#endif /*_TOPIC_MONITOR_LOG_HPP_*/
