# Text Resource Modifier

**Author**: TheRedDaemon  
**Version**: 0.3.0  
**Repository**: [https://github.com/TheRedDaemon/ucp_textResourceModifier](https://github.com/TheRedDaemon/ucp_textResourceModifier)

This module exposes the possibility to modify the text returned for the internal text resource. Changing localized text is already possible by editing the `cr.tex` file. However, these changes can not happen dynamically during runtime and it makes partial modifications harder.

This module intends to provide the functionality to change specific texts while also allowing to provide the text as UTF-8, which allows a common text format. The module uses the single text receiver function of Stronghold Crusader for this. It is detoured to a function that uses the index and the number in this index of the requested string to generate a key. If a string was registered for it, the custom string is returned, otherwise it falls back to the default handler.

The API to register strings expects UTF-8 and transforms it to the current encoding of the game.

This module currently provides no direct features and functions therefore as a low level extension. For more infos, please check out the readme on the repository.