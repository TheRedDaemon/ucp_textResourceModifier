# UCP Text Resource Modifier

This repository contains a module for the "Unofficial Crusader Patch Version 3" (UCP3), a modification for Stronghold Crusader.
This module exposes the possibility to modify the text returned for the internal text resource.


### Motivation and Plan

Changing localized text is already possible by editing the `cr.tex` file.
However, these changes can not happen dynamically during runtime and it makes partial modifications harder.
Additionally, the text file is not a simple text file and instead uses special encoding, possibly the windows internal 16-bit unicode. 

This module intends to provide the functionality to change specific texts while also allowing to provide the text as UTF-8, which allows a common text format.
The module uses the single text receiver function of Stronghold Crusader for this.
It is detoured to a function that uses the index and the number in this index of the requested string to generate a key.
If a string was registered for it, the custom string is returned, otherwise it falls back to the default handler.

The API toe register strings expects UTF-8 and transforms it to the current encoding of the game.


### Usage

The module is part of the UCP3. Certain commits of the main branch of this repository are included as a git submodule.
It is therefore not needed to download additional content.

However, should issues or suggestions arise that are related to this module, feel free to add a new GitHub issue.
Support is currently only guaranteed for the western versions of Crusader 1.41 and Crusader Extreme 1.41.1-E.
Other, eastern versions of 1.41 might work.


### C++-Exports

At the time of creation, C functions would need to be parsed through lua in the form of numbers. To make using the module easier, the header [textResourceModifierHeader.h](ucp_textResourceModifier/ucp_textResourceModifier/textResourceModifierHeader.h) can be copied into your project.
It is used by calling the function `initModuleFunctions(lua_state * )` during the lua require call of the dll. It tries to receive the provided functions and returns `true` if successful. For this to work, the *textResourceModifier* needs to be a dependency of your module.
The provided functions are the following:


* `bool SetText(int offsetIndex, int numInGroup, const char* utf8Str)`  
  Allows to register an UTF-8-string for a specific combination of `offsetIndex` and  `numInGroup`.
  If the string pointer is `null`, the registered string is removed and the native handler will once again be used.
  Returns `true` if the action was successful.
  Note, that strings registered before start-up will be transformed after the games own initialization.
  If they fail there, they are removed and an error is printed to the console.

* `const char* GetText(int offsetIndex, int numInGroup)`  
  Returns the current string for the given `offsetIndex` and `numInGroup`.
  This might either be the games own string or the current overwriting string.
  Only works after the game text is first loaded.
  Note, that there are no safety checks, since this module should also allow the register strings on new indexes.
  Use with caution, either for tests, or if you are sure this index exits.

* `const char* GetLanguage()`  
  Simply returns the language string for the loaded text source file.
  Only works after the game text is first loaded.

* `void TransformText(const char* utf8Str, TextReceiver* receiver, void* misc)`  
  Receives an UTF-8 string and tries to transform it into the games text encoding.
  Can be used for text if it should not be managed by this module.
  The `receiver` is used to return the transformed string or a dummy string on failure.
  `misc` is passed through to the receiver function.
  Only works after the game text is first loaded.

  * `void TextReceiver(const char* transformedString, void* misc)`  
    The structure of the function pointer needed by `TransformText`.
    Is called with the transformed string and the `misc` given before.


### Lua-Exports

The Lua exports are parameters and functions accessible through the module object.

* `bool SetText(int offsetIndex, int numInGroup, string utf8Str)`  
  Allows to register an UTF-8-string for a specific combination of `offsetIndex` and  `numInGroup`.
  If the string is `nil`, the registered string is removed and the native handler will once again be used.
  Returns `true` if the action was successful.
  Note, that strings registered before start-up will be transformed after the games own initialization.
  If they fail there, they are removed and an error is printed to the console.

* `string GetText(int offsetIndex, int numInGroup)`  
  Returns the current string for the given `offsetIndex` and `numInGroup`.
  This might either be the games own string or the current overwriting string.
  Only works after the game text is first loaded.
  Note, that there are no safety checks, since this module should also allow the register strings on new indexes.
  Use with caution, either for tests, or if you are sure this index exits.

* `string GetLanguage()`  
  Simply returns the language string for the loaded text source file.
  Only works after the game text is first loaded.

* `string TransformText(string utf8Str)`  
  Receives an UTF-8 string and tries to transform it into the games text encoding.
  Can be used for text if it should not be managed by this module.
  Only works after the game text is first loaded.


### Special Thanks

To all of the UCP Team, the [Ghidra project](https://github.com/NationalSecurityAgency/ghidra) and
of course to [Firefly Studios](https://fireflyworlds.com/), the creators of Stronghold Crusader.
