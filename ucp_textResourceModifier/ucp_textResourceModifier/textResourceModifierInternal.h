#pragma once

#include <string>
#include <unordered_map>

#include "textResourceModifierHeader.h"

namespace TRMH = TextResourceModifierHeader; // easier to use

/* classes and objects */

// class used to link this calls to
class TextHandler
{
public:
  using FuncLoadCRTex = void (TextHandler::*)();

  inline static FuncLoadCRTex realLoadCRTFunc{};
  inline static DWORD toNativeTextAddr{};

private:

  inline static TextHandler* textManagerPtr{ nullptr }; // used to get text using this module
  inline static int gameCodepage{};
  inline static std::unordered_map<int, std::string> stringMap{};

public:

  static bool SetText(int offsetIndex, int numInGroup, const char* utf8Str);
  static const char* GetText(int offsetIndex, int numInGroup);
  static std::string TransformText(const char* utf8Str);

  // required to set function
  // important: will not receive actual ptr to an object of this class
  void __thiscall interceptedLoadCRTex();
  const char* __thiscall getMapText(int offsetIndex, int numInGroup);

private:

  // logs the error, but leaves the string in an unchanged state
  static void LogTranscodingError();
  static bool TransformUTF8ToSHCLocale(const char* utf8Str, std::string& strToAssignResTo);

  // game called functions
  // IMPORTANT: none of these functions will receive a ptr to this class, but to the TextManager class of SHC

  const char* __thiscall getNativeText(int offsetIndex, int numInGroup);  // needs to be implemented as naked
};


/* functions */

/* exports */

extern "C" __declspec(dllexport) bool __stdcall SetText(int offsetIndex, int numInGroup, const char* utf8Str);
extern "C" __declspec(dllexport) const char* __stdcall GetText(int offsetIndex, int numInGroup);
extern "C" __declspec(dllexport) void __stdcall TransformText(const char* utf8Str, TRMH::FuncTextReceiver receiver, void* misc);
extern "C" __declspec(dllexport) const char* __stdcall GetLanguage();

/* LUA */

extern "C" __declspec(dllexport) int __cdecl lua_SetText(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_GetText(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_TransformText(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_GetLanguage(lua_State * L);