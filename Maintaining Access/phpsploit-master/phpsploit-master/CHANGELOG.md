### Version 2.3.0 *(IN DEVELOPMENT)*
- Fix issue #9 (small bug in api.payload.Payload())
- `lrun` command is now able to change PWD (issue #10)
- Remove deprecated `lcd` and `lpwd` commands.
- Fix some small bugs and documentation misspells.
- Fix issue #6 (*_proxy env var handling through http tunnel).
- All settings can now be reset with `set <VAR> %%DEFAULT%%`
- Add full backward compatibility with older phpsploit session files.
- Fix issue #1 (the `ls` plugin leaves at first invalid path)
- Fix no existing file in datatypes/Path
- Fix issue #5 - Add a '--browser' option to `phpinfo` plugin for html display. 
### Version 2.2.0b *(2014-08-09)*
- **Rewritten the whole PhpSploit framework** in python 3 with new skeleton.
- The `system` have been renamed into `run`.
- Add `corectl` command, which includes some core debugging utils.
- `TEXTEDITOR` setting has been renamed to `EDITOR`.
- `WEBBROWSER` setting has been renamed to `BROWSER`.
- The `infect` command has been removed, its role is now taken by `exploit`.
- The new `session` command now manages the old `load` and `save` commands.
- The `set` command now supplies a new keyword ("+") for line appending.
- Any setting now suports random choice from multiple values, with the new
  `set` command's `+` keyword, that uses SettingVar class as data wrapper.
- The `eval` command has been replaced by `source`, more restrictive.
- The `lastcmd` command has been replaced by `backlog`, more simple.
- The phpsploit source code has moved to **./src/** directory.
- Plugins path is now available at root directory.
- User plugins can now overwrite core plugins (**~/.phpsploit/plugins/**)

### Version 2.1.4 *(2013-04-07)*
- The plugins main script names are now **plugin.py** instead of **script.py**.
- Plugin help strings are now taken from the plugin.py's docString.
- Improved all command specific help strings, with man like syntax.
- New `load` command, which provides interactive session file loading.
- Typing `<cmd> --help` now shows help as `help <cmd>`.
- Prefixing a command with % from virtual plugin **shell** instances allows
  remoteShell command execution without leaving the virtual shell.
- Added lowercase variable name completion on `set` and `env` commands.
- The **./doc/** files has been moved to the root directory.

### Version 2.1.3 *(2013-02-20)*
- The **system** plugin now handles empty responses.
- If the loaded session has changed, shell leaving must now be confirmed.
- The `env` and `set` commands now display variables which names starts
- with the first argument string if it is not a complete variable name.
- If present, the bash **$XDG_CONFIG_HOME** environment variable is now used
  as user's root configuration path instead of home directory.
- **./framework/misc/** is now the default miscellaneous files directory.
- The ascii **README** has been moved to the PhpSploit root directory.
- **./doc/** is now used as help files containers instead of **./readme/**.
- Added a **./doc/CONTRIBUTE** file, which is a readme for contributors.
- The **DISCLAIMER** has been fully rewritten for legal purposes.
- The user manual (man) template is now available at **./framework/misc/**.
- Settings from loaded session are now correctly checked at load.
- Added multi command support, with semicolon separator (bash like).
- Command arguments now support bash like enquoting features.
- Added the `eval` command, providing commands list execution from file.

### Version 2.1.2 *(2012-08-12)*
- The commands can now be written in multi line mode, like in the bash
- interpreter, by ending a line with an antislash.
- Pressing enter on an unfinished enquoted command is now interpreted as
  a newline insertion instead of a command sending, like in bash.
