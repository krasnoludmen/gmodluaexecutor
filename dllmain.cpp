#pragma comment(lib, "libMinHook.x86.lib")
#include <Windows.h>
#include <MinHook.h>
#include <iostream>
#include <fstream>
#include <regex>
struct lua_State;
typedef intptr_t lua_KContext;
typedef int (*lua_KFunction) (lua_State* L, int status, lua_KContext ctx);

using luaL_loadbufferx_t = int(*)(lua_State*, char*, size_t, char*, const char*);
using lua_pcall_t = int(*)(lua_State*, int, int, int, lua_KContext, lua_KFunction);

HMODULE luaBase = GetModuleHandleA("lua_shared.dll");

auto luaL_loadbufferx = (luaL_loadbufferx_t)(GetProcAddress(luaBase, "luaL_loadbufferx"));
auto lua_pcall = (lua_pcall_t)(GetProcAddress(luaBase, "lua_pcall"));

luaL_loadbufferx_t o_luaL_loadbufferx = nullptr;

lua_State* state;

int luaL_loadbufferx_hook(lua_State* L, char* buff, size_t size, char* name, const char* mode) {
    state = L;
    printf("%s - %s\n", name, buff);
    return o_luaL_loadbufferx(L, buff, size, name, mode);
}


void LoadLua(std::string buff) {
    if (o_luaL_loadbufferx(state, const_cast<char*>(buff.c_str()), buff.length(), const_cast<char*>("test.lua"), "t") != 0) {
        printf("error nigger\n");
    }
    if (lua_pcall(state, 0, 0, 0, 0, 0) != 0) {
        printf("erorr nigger2\n");
    }
}


void Init() {
    AllocConsole();
    freopen_s((FILE**)stdin, "conin$", "r", stdin);
    freopen_s((FILE**)stdout, "conout$", "w", stdout);
    MH_Initialize();
    MH_CreateHook((LPVOID)luaL_loadbufferx, &luaL_loadbufferx_hook, (LPVOID*)&o_luaL_loadbufferx);
    MH_EnableHook(MH_ALL_HOOKS);
    while (true) {
        std::string cmd;
        std::getline(std::cin, cmd);
        LoadLua(cmd);
    }
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    BOOL Injected = false;
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Init, 0, 0, 0);
        Injected = true;
    }
    return Injected;
}

