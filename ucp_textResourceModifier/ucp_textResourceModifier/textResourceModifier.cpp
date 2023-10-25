
#include "pch.h"

#include "textResourceModifierInternal.h"

#include <vector>
#include <system_error>



bool TextHandler::SetText(int offsetIndex, int numInGroup, const char* utf8Str)
{
  int mapIndex{ (offsetIndex << 16) | (numInGroup & 0x0000FFFF) };  // offset is higher order short, number in group lower (I think)

  // important -> check if valid value exists, because if not, the entry needs to be deleted again
  auto iter{ stringMap.find(mapIndex) };
  if (iter == stringMap.end())
  {
    if (!utf8Str)
    {
      return true;  // no string present, so everything ok
    }

    if (!gameCodepage)  // simply construct string
    {
      auto res{ stringMap.try_emplace(mapIndex, utf8Str) };
      return res.second;  // if insertion happend
    }

    std::string tempStr;
    if (TransformUTF8ToSHCLocale(utf8Str, tempStr))
    {
      auto res{ stringMap.try_emplace(mapIndex, std::move(tempStr)) };  // moves the string, since we do not need it here any more
      return res.second;  // if insertion happend
    }
    return false; // do nothing, since no conversion
  }

  // is present:

  if (!utf8Str)
  {
    stringMap.erase(iter);  // remove
    return true;
  }

  if (!gameCodepage)  // simply assign string
  {
    iter->second = utf8Str;
    return true;  // if insertion happend
  }

  // true if changed, false else -> string is only modified if transform successful
  return TransformUTF8ToSHCLocale(utf8Str, iter->second);
}


const char* TextHandler::GetText(int offsetIndex, int numInGroup)
{
  if (!textManagerPtr)
  {
    Log(LOG_WARNING, "[textResourceModifier]: Unable to get text. CRTex not yet loaded.");
    return "ERROR: NO TEXT RESOURCE YET LOADED.";
  }
  return textManagerPtr->getMapText(offsetIndex, numInGroup);
}

std::string TextHandler::TransformText(const char* utf8Str)
{
  if (!textManagerPtr)
  {
    Log(LOG_WARNING, "[textResourceModifier]: Unable to transform text. CRTex not yet loaded.");
    return "ERROR: NO TEXT RESOURCE YET LOADED.";
  }

  std::string strContainer{ "ERROR: FAILED TO TRANSFORM TEXT." };
  if (!TransformUTF8ToSHCLocale(utf8Str, strContainer))
  {
    Log(LOG_WARNING, "[textResourceModifier]: Unable to transform text. Transformation failed.");
    return strContainer;
  }
  return strContainer;
}


// source: https://stackoverflow.com/a/62070624
// apparently not without issues, but hopefully save enough
void TextHandler::LogTranscodingError()
{
  DWORD lastError{ GetLastError() };
  Log(LOG_WARNING, ("[textResourceModifier]: Failed transcoding: " + std::system_category().message(lastError)).c_str());
}


// the ptr needs to point to a valid c string, and these are never truly empty (/0)
bool TextHandler::TransformUTF8ToSHCLocale(const char* utf8Str, std::string& strToAssignResTo)
{
  std::vector<WCHAR> wCharBuffer{};
  std::vector<char> resCharBuffer{};

  int sizeAndRes{ MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, 0, 0) };
  if (!sizeAndRes)
  {
    LogTranscodingError();
    return false;
  }
  wCharBuffer.resize(sizeAndRes);

  sizeAndRes = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wCharBuffer.data(), sizeAndRes);
  if (!sizeAndRes)
  {
    LogTranscodingError();
    return false;
  }

  // this second count might not be needed, but lets do it for safety
  sizeAndRes = WideCharToMultiByte(gameCodepage, 0, wCharBuffer.data(), -1, 0, 0, 0, 0);
  if (!sizeAndRes)
  {
    LogTranscodingError();
    return false;
  }
  resCharBuffer.resize(sizeAndRes);

  // using system default char
  sizeAndRes = WideCharToMultiByte(gameCodepage, 0, wCharBuffer.data(), -1, resCharBuffer.data(), sizeAndRes, 0, 0);
  if (!sizeAndRes)
  {
    LogTranscodingError();
    return false;
  }

  strToAssignResTo = resCharBuffer.data();
  return true;
}



/* fake "this" functions */

// "this" will be the games text manager
void __thiscall TextHandler::interceptedLoadCRTex()
{
  textManagerPtr = this;

  // call actual load function
  (*textManagerPtr.*realLoadCRTFunc)();

  // read codepage value
  gameCodepage = *((int*)textManagerPtr + 4); // fifth dword in class

  if (!gameCodepage)
  {
    Log(LOG_ERROR, "[textResourceModifier]: Failed to get games codepage. No text changes will be applied.");
  }

  // transform all given strings so far
  auto it = stringMap.begin();
  while (it != stringMap.end())
  {
    if (TransformUTF8ToSHCLocale(it->second.c_str(), it->second))
    {
      ++it;
    }
    else
    {
      // remove if transform failed
      it = stringMap.erase(it);
    }
  }
}


const char* __thiscall TextHandler::getMapText(int offsetIndex, int numInGroup)
{
  if (!gameCodepage)
  {
    return this->getNativeText(offsetIndex, numInGroup);
  }

  int mapIndex{ (offsetIndex << 16) | (numInGroup & 0x0000FFFF) };  // offset is higher order short, number in group lower (I think)

  auto iter{ stringMap.find(mapIndex) };
  return (iter == stringMap.end()) ? this->getNativeText(offsetIndex, numInGroup) : iter->second.c_str();
}


__declspec(naked) const char* __thiscall TextHandler::getNativeText(int offsetIndex, int numInGroup)
{
  __asm {
    mov eax, dword ptr[esp + 0x4] // replicate the overwritten instructions
    lea edx, [eax + -0x5]
    jmp toNativeTextAddr  // jump to real code
  }
}



/* export C */

extern "C" __declspec(dllexport) bool __stdcall SetText(int offsetIndex, int numInGroup, const char* utf8Str)
{
  return TextHandler::SetText(offsetIndex, numInGroup, utf8Str);
}

extern "C" __declspec(dllexport) const char* __stdcall GetText(int offsetIndex, int numInGroup)
{
  return TextHandler::GetText(offsetIndex, numInGroup);
}

extern "C" __declspec(dllexport) void __stdcall TransformText(const char* utf8Str, TRMH::FuncTextReceiver receiver, void* misc)
{
  std::string holder{ TextHandler::TransformText(utf8Str) };
  receiver(holder.c_str(), misc);
}

extern "C" __declspec(dllexport) const char* __stdcall GetLanguage()
{
  return TextHandler::GetText(6, 0);
}


/* export LUA */

extern "C" __declspec(dllexport) int __cdecl lua_SetText(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 3)
  {
    luaL_error(L, "[textResourceModifier]: lua_SetText: Invalid number of args.");
  }

  if (!(lua_isinteger(L, 1) && lua_isinteger(L, 2) && (lua_isstring(L, 3) || lua_isnoneornil(L, 3))))
  {
    luaL_error(L, "[textResourceModifier]: lua_SetText: Wrong input fields.");
  }

  bool res{ TextHandler::SetText(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tostring(L, 3)) };
  lua_pushboolean(L, res);
  return 1;
}

extern "C" __declspec(dllexport) int __cdecl lua_GetText(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 2)
  {
    luaL_error(L, "[textResourceModifier]: lua_GetText: Invalid number of args.");
  }

  if (!(lua_isinteger(L, 1) && lua_isinteger(L, 2)))
  {
    luaL_error(L, "[textResourceModifier]: lua_GetText: Wrong input fields.");
  }

  lua_pushstring(L, TextHandler::GetText(lua_tointeger(L, 1), lua_tointeger(L, 2)));
  return 1;
}

extern "C" __declspec(dllexport) int __cdecl lua_TransformText(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 1)
  {
    luaL_error(L, "[textResourceModifier]: lua_TransformText: Invalid number of args.");
  }

  if (!lua_isstring(L, 1))
  {
    luaL_error(L, "[textResourceModifier]: lua_TransformText: Wrong input fields.");
  }

  lua_pushstring(L, TextHandler::TransformText(lua_tostring(L, 1)).c_str());
  return 1;
}

extern "C" __declspec(dllexport) int __cdecl lua_GetLanguage(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 0)
  {
    luaL_error(L, "[textResourceModifier]: lua_GetLanguage: Invalid number of args.");
  }

  lua_pushstring(L, GetLanguage());
  return 1;
}
