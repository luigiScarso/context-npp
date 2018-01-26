# ConTeXt

This is a notepad++ plugin for ConTeXt.

This plugin have 3 major functions:
1. Provide syntax lexing and folding for ConTeXt files.
2. Provide "webedit" functions for ConTeXt.
3. Options to add "webedit" functions in context menu (right-click menu).

Lexing and folding:
This lexer provides 5 keyword groups/styles for syling, and 2 additional groups for folding. All of the default keywords and associated extensions (default as .tex) can be edited in the related XML file (normally found in C:\Program Files (x86)\Notepad++\plugins\Config\ConTeXt.xml).

WebEdit:
WebEdit functions are defined in ini file (normally found in C:\Users\Jason\AppData\Roaming\Notepad++\plugins\config\ConTeXtEdit.ini). 
Commands are wrapped in two tags: [Commands] and [CommandsEnd]. Inside these, one user can specify keyword groups by adding group name inside brackets [], and these will be used as the submenu names. Under each group name, the keywords can be specified in the following syntax: Syntax: <Item name>=<Left text>|<Right text>. In order to add separator, one can add '---' between keywords. 

Right-click menu:
Because of internal defects in Notepad++, right-click menu implementation is not very straightforward. To add context menus, one can click 'Update Context Menu' to modify the contextmenu.xml. After restarting NPP, context menus will show up. One can also remove all user-defined context menu by clicking "Remove Context Menu" and restarting NPP.