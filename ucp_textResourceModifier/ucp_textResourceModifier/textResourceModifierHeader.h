
#ifndef TEXT_RESOURCE_MODIFIER_HEADER
#define TEXT_RESOURCE_MODIFIER_HEADER

#include <lua.hpp>

namespace TextResourceModifierHeader
{
  // Cpp API

  using FuncSetText = bool(__stdcall*)(int offsetIndex, int numInGroup, const char* utf8Str);
  using FuncGetText = const char*(__stdcall*)(int offsetIndex, int numInGroup);

  inline constexpr char const* NAME_VERSION{ "0.2.0" };

  inline constexpr char const* NAME_MODULE{ "textResourceModifier" };
  inline constexpr char const* NAME_SET_TEXT{ "_SetText@12" };
  inline constexpr char const* NAME_GET_TEXT{ "_GetText@8" };

  inline FuncSetText SetText{ nullptr };
  inline FuncGetText GetText{ nullptr };

  // returns true if the function variables of this header were successfully filled
  inline bool initModuleFunctions(lua_State* L)
  {
    if (SetText && GetText) // assumed to not change during runtime
    {
      return true;
    }

    if (lua_getglobal(L, "modules") != LUA_TTABLE)
    {
      lua_pop(L, 1);  // remove value
      return false;
    }

    if (lua_getfield(L, -1, NAME_MODULE) != LUA_TTABLE)
    {
      lua_pop(L, 2);  // remove table and value
      return false;
    }

    GetText = (lua_getfield(L, -1, NAME_GET_TEXT) == LUA_TNUMBER) ? (FuncGetText)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 1);
    SetText = (lua_getfield(L, -1, NAME_SET_TEXT) == LUA_TNUMBER) ? (FuncSetText)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 3);  // remove value and all tables

    return SetText && GetText;
  }
}

#endif //TEXT_RESOURCE_MODIFIER_HEADER