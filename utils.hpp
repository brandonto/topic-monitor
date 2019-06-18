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
#ifndef _TOPIC_MONITOR_UTILS_HPP_
#define _TOPIC_MONITOR_UTILS_HPP_

#include <lua5.2/lua.hpp>

namespace topicMonitor
{

namespace luaUtils
{
    const char*
    getStringValueFromSymbol(lua_State* L, const char* symbol_p)
    {
        // Save the current stack size
        //
        int luaStackSize = lua_gettop(L);

        // Push value of the symbol on the stack: if it exists, the stack size
        // will increase by 1
        //
        lua_getglobal(L, symbol_p);
        if (lua_gettop(L) != luaStackSize + 1) { return nullptr; }

        // Get the value of the symbol from the stack
        //
        if (!lua_isstring(L, -1)) { return nullptr; }
        const char* value_p = lua_tostring(L, -1);

        // Pop the value of the symbol to leave stack in original state
        //
        lua_pop(L, 1);

        return value_p;
    }
} /* namespace luaUtils */

} /* namespace topicMonitor */

#endif /* _TOPIC_MONITOR_UTILS_HPP_ */
