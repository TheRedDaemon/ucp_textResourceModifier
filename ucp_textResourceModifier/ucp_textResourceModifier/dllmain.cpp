
#include "pch.h"

// lua
#include "lua.hpp"

#include "textResourceModifierInternal.h"

// lua module load
extern "C" __declspec(dllexport) int __cdecl luaopen_textResourceModifier(lua_State * L)
{
  if (!LuaLog::init(L))
  {
    luaL_error(L, "[InputHandler]: Failed to receive Log functions.");
  }

  lua_newtable(L); // push a new table on the stack

  // simple replace
  // get member func ptr, source: https://github.com/microsoft/Detours/blob/master/samples/member/member.cpp
  auto memberFuncPtr{ &TextHandler::interceptedLoadCRTex };
  lua_pushinteger(L, *(DWORD*)&memberFuncPtr);
  lua_setfield(L, -2, "funcAddress_InterceptedLoadCRTex");

  // replace at start of func and jump to this
  auto memberFuncPtr2{ &TextHandler::getMapText };
  lua_pushinteger(L, *(DWORD*)&memberFuncPtr2);
  lua_setfield(L, -2, "funcAddress_GetMapText");

  // address
  lua_pushinteger(L, (DWORD)&TextHandler::realLoadCRTFunc);
  lua_setfield(L, -2, "address_RealLoadCRTFuncAddr");

  // address
  lua_pushinteger(L, (DWORD)&TextHandler::toNativeTextAddr);
  lua_setfield(L, -2, "address_ToNativeTextAddr");

  // add functions
  lua_newtable(L); // push function table
  lua_pushinteger(L, (DWORD)SetText);
  lua_setfield(L, -2, TextResourceModifierHeader::NAME_SET_TEXT);

  // add table
  lua_setfield(L, -2, "funcPtr");

  // return lua funcs

  lua_pushcfunction(L, lua_SetText);
  lua_setfield(L, -2, "lua_SetText");

  return 1;
}

// entry point
BOOL APIENTRY DllMain(HMODULE,
  DWORD  ul_reason_for_call,
  LPVOID
)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}