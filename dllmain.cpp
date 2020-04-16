#pragma comment(lib, "libMinHook.x86.lib")
#include <Windows.h>
#include <MinHook.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
struct lua_State;
typedef intptr_t lua_KContext;
typedef int (*lua_KFunction) (lua_State* L, int status, lua_KContext ctx);

using luaL_loadbufferx_t = int(*)(lua_State*, const char*, size_t, const char*, const char*);
using lua_pcall_t = int(*)(lua_State*, int, int, int, lua_KContext, lua_KFunction);
using lua_settop_t = void(*)(lua_State*, int);
using luaL_checklstring_t = const char*(*)(lua_State*, int, size_t);

HMODULE luaBase = GetModuleHandleA("lua_shared.dll");

auto luaL_loadbufferx = (luaL_loadbufferx_t)(GetProcAddress(luaBase, "luaL_loadbufferx"));
auto lua_pcall = (lua_pcall_t)(GetProcAddress(luaBase, "lua_pcall"));
auto lua_settop = (lua_settop_t)(GetProcAddress(luaBase, "lua_settop"));
auto luaL_checklstring = (luaL_checklstring_t)(GetProcAddress(luaBase, "luaL_checklstring"));
luaL_loadbufferx_t o_luaL_loadbufferx = nullptr;

lua_State* state;

#define lua_pop(L,n) lua_settop(L, -(n)-1);
#define lua_checkstring(L, n) (luaL_checklstring(L, (n), NULL))


void ReplaceStringInPlace(std::string& subject, const std::string& search,
    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

std::string ReadFromFile(std::string path) {
    if (path.find("/") != std::string::npos) {
        ReplaceStringInPlace(path, "/", "\\");
    }
    std::ifstream file(path);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}


int luaL_loadbufferx_hook(lua_State* L, char* buff, size_t size, char* name, const char* mode) {
    state = L;
    if (std::filesystem::exists("C:\\gmod dump") == false) {
        std::filesystem::create_directory("C:\\gmod dump");
    }
    std::ofstream file;
    std::string filename(name);
    if (filename.find("/") != std::string::npos) {
        ReplaceStringInPlace(filename, "/", "@");
    }
    file.open(std::string("C:\\gmod dump\\").append(filename.c_str()));
    file << buff;
    file.close();
    return o_luaL_loadbufferx(L, buff, size, name, mode);
}

void loadLua(std::string buff) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (o_luaL_loadbufferx(state, buff.c_str(), buff.length(), "memerino.lua", "t") != 0) {
        std::string err = lua_checkstring(state, -1);
        lua_pop(state, 1);
        SetConsoleTextAttribute(console, 12);
        printf("error parsing script - %s\n", err.c_str());
        SetConsoleTextAttribute(console, 7);
        return;
    }
    if (lua_pcall(state, 0, 0, 0, 0, 0) != 0) {
        std::string err = lua_checkstring(state, -1);
        lua_pop(state, 1);
        SetConsoleTextAttribute(console, 12);
        printf("error loading script - %s\n", err.c_str());
        SetConsoleTextAttribute(console, 7);
        return;
    }
    printf("executed.\n");
}


void init() {
    AllocConsole();
    SetConsoleTitleA("Garry's Mod Lua Executor @ created by krasnolud#5823");
    freopen_s((FILE**)stdin, "conin$", "r", stdin);
    freopen_s((FILE**)stdout, "conout$", "w", stdout);
    MH_Initialize();
    MH_CreateHook((LPVOID)luaL_loadbufferx, &luaL_loadbufferx_hook, (LPVOID*)&o_luaL_loadbufferx);
    MH_EnableHook(MH_ALL_HOOKS);
    while (true) {
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd.find("file") != std::string::npos) {
            ReplaceStringInPlace(cmd, "file ", "");
            cmd = ReadFromFile(cmd);
        }
        else if (cmd.find("clear") != std::string::npos) { system("cls"); continue; }
        loadLua(cmd);
    }
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    BOOL injected = false;
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init, 0, 0, 0);
        injected = true;
    }
    return injected;
}

