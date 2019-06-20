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
#include "utils.hpp"

namespace topicMonitor
{
namespace utils
{

std::string
lua::getStringValueFromSymbol(lua_State* L, std::string symbol)
{
    // Save the current stack size
    //
    int luaStackSize = lua_gettop(L);

    // Push value of the symbol on the stack: if it exists, the stack size will
    // increase by 1
    //
    lua_getglobal(L, symbol.c_str());
    if (lua_gettop(L) != luaStackSize + 1) { return nullptr; }

    // Get the value of the symbol from the stack
    //
    if (!lua_isstring(L, -1)) { return nullptr; }
    std::string value = lua_tostring(L, -1);

    // Pop the value of the symbol to return stack in original state
    //
    lua_pop(L, 1);

    return value;
}

// Load lua script into a lua env
//
// https://stackoverflow.com/questions/36356498/multiple-scripts-in-a-single-lua-state-and-working-with-env
//
returnCode_t
lua::loadFileInEnv(lua_State* L, std::string filename, std::string env)
{
    // TODO (BTO): Consider making this a configurable path
    //
    std::string filepath = "monitoring-scripts/" + filename;
    if (luaL_loadfile(L, filepath.c_str()) != 0)
    {
        return returnCode_t::FAILURE;
    }

    lua_newtable(L); // Create table T1 for setting up _ENV table, pushes T1
    lua_newtable(L); // Create table T2 for setting up metatable, pushes T1
    lua_getglobal(L, "_G"); // Pushes global table _G
    lua_setfield(L, -2, "__index"); // T2["__index"] = "_G", pops "_G"
    lua_setmetatable(L, -2); // T1 = T2, pops T2
    lua_setfield(L, LUA_REGISTRYINDEX, env.c_str()); // REGISTRY[env_p] = T1, pops T1
    lua_getfield(L, LUA_REGISTRYINDEX, env.c_str()); // Pushes T1
    lua_setupvalue(L, 1, 1); // TODO (BTO): What the heck does this do????
    lua_pcall(L, 0, LUA_MULTRET, 0); // Runs the file in the lua env

    return returnCode_t::SUCCESS;
}

bool
lua::isFuncInEnv(lua_State* L, std::string env, std::string func)
{
    lua_getfield(L, LUA_REGISTRYINDEX, env.c_str());
    lua_getfield(L, -1, func.c_str());
    bool isFunction = lua_isfunction(L, -1);
    lua_pop(L, 2); // Leave stack in original state
    return isFunction;
}

returnCode_t
lua::callMessageFunc(lua_State* L, std::string env, std::string data)
{
    lua_getfield(L, LUA_REGISTRYINDEX, env.c_str());
    lua_getfield(L, -1, LUA_MESSAGE_FUNC);
    lua_pushstring(L, data.c_str());
    if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    {
        return returnCode_t::FAILURE;
    }

    return returnCode_t::SUCCESS;
}

void
lua::stackTrace(lua_State *L)
{
    int i;
    int top = lua_gettop(L);
    printf("---- Begin Stack ----\n");
    printf("Stack size: %i\n\n", top);
    for (i = top; i >= 1; i--)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING:
            printf("%i -- (%i) ---- `%s'", i, i - (top + 1), lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%i -- (%i) ---- %s", i, i - (top + 1), lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            printf("%i -- (%i) ---- %g", i, i - (top + 1), lua_tonumber(L, i));
            break;
        default:
            printf("%i -- (%i) ---- %s", i, i - (top + 1), lua_typename(L, t));
            break;
        }
        printf("\n");
    }
    printf("---- End Stack ----\n");
    printf("\n");
}

} /* namespace utils */
} /* namespace topicMonitor */
