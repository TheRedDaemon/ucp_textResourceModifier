
#include "pch.h"

// lua
#include "lua.hpp"

#include "textResourceModifierInternal.h"

// lua module load
extern "C" __declspec(dllexport) int __cdecl luaopen_textResourceModifier(lua_State * L)
{
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

  // return lua funcs

  lua_pushcfunction(L, lua_SetText);
  lua_setfield(L, -2, "lua_SetText");
  lua_pushcfunction(L, lua_GetText);
  lua_setfield(L, -2, "lua_GetText");
  lua_pushcfunction(L, lua_TransformText);
  lua_setfield(L, -2, "lua_TransformText");
  lua_pushcfunction(L, lua_GetLanguage);
  lua_setfield(L, -2, "lua_GetLanguage");

  return 1;
}