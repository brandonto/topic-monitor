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

const char*
lua::getStringValueFromSymbol(lua_State* L, const char* symbol_p)
{
    // Save the current stack size
    //
    int luaStackSize = lua_gettop(L);

    // Push value of the symbol on the stack: if it exists, the stack size will
    // increase by 1
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

// Load lua script into a lua env
//
// https://stackoverflow.com/questions/36356498/multiple-scripts-in-a-single-lua-state-and-working-with-env
//
returnCode_t
lua::loadFileInEnv(lua_State* L, const char* filename_p, const char* env_p)
{
    // TODO (BTO): Consider making this a configurable path
    //
    static const char* SCRIPT_DIRECTORY = "monitoring-scripts/";
    static const size_t SCRIPT_DIRECTORY_SIZE = strlen(SCRIPT_DIRECTORY);

    char filepath_a[MAX_FILENAME_SIZE + SCRIPT_DIRECTORY_SIZE + 1];
    strcpy(filepath_a, SCRIPT_DIRECTORY);
    strcat(filepath_a, filename_p);

    if (luaL_loadfile(L, filepath_a) != 0)
    {
        return returnCode_t::FAILURE;
    }

    lua_newtable(L); // Create table T1 for setting up _ENV table, pushes T1
    lua_newtable(L); // Create table T2 for setting up metatable, pushes T1
    lua_getglobal(L, "_G"); // Pushes global table _G
    lua_setfield(L, -2, "__index"); // T2["__index"] = "_G", pops "_G"
    lua_setmetatable(L, -2); // T1 = T2, pops T2
    lua_setfield(L, LUA_REGISTRYINDEX, env_p); // REGISTRY[env_p] = T1, pops T1
    lua_getfield(L, LUA_REGISTRYINDEX, env_p); // Pushes T1
    lua_setupvalue(L, 1, 1); // TODO (BTO): What the heck does this do????
    lua_pcall(L, 0, LUA_MULTRET, 0); // Runs the file in the lua env

    return returnCode_t::SUCCESS;
}

// TODO (BTO): This is just a proof of concept... Fix this entire function
//
returnCode_t
lua::callFuncInEnv(lua_State* L, const char* functionName_p, const char* env_p)
{
    lua_getfield(L, LUA_REGISTRYINDEX, env_p);
    lua_getfield(L, -1, functionName_p);
    lua_call(L, 0, 0);

    return returnCode_t::SUCCESS;
}

} /* namespace utils */
} /* namespace topicMonitor */
