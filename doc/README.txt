This plugin aims to support the editing of ConTeXt documents by integrating the
files included in the standalone distribution (for more information on ConTeXt
see http://wiki.contextgarden.net/Main_Page and
http://www.pragma-ade.nl/index.htm). In the first phase the plugin borrowed from
other plugins (notably WebEdit) some ideas and implementations, and from the
experience of Prof. Hamid several macros and customization; in this second step
a significant part of development was spent to translate the documentation files
in xml format of ConTeXt (the interface files) into the xml files of Notepad++
and to look for an efficient way to edit a document trying to minimize the
access to the documentation.

One of the fundamental difference between ConTeXt and a traditional programming
language is the huge amount of keywords (the macros) and parameters (the
arguments) of ConTeXt, but fortunately almost all of them are described in the
interface files of the standalone distribution, under
tex\texmf-context\tex\context\interface\mkiv. Here the file context-en.xml is
the concatenation of all the others interface files.

The build.cmd script under the scripts folder of Npp-for-ConTeXt.zip read
context-en.xml and produces as result two files, ConTeXt.xml for the lexer of
Notepad++ and APIs/context.xml for the auto-completion and call-tip. The build.cmd
script is a batch script that has to be executed outside Notepad++; under the
hood it runs the Python2.7 script command_primitives_api.py with the correct
command line options, and if the xmllint.exe is installed it checks if the files
saved are well-formed.

The Npp-for-ConTeXt.zip already has the ConTeXt.xml and APIs/context.xml files
because it has as reference the ConTeXt as shipped in the TeXLive 2018 distribution
(see https://www.tug.org/texlive/).

The other component introduced is the User's macro window, enabled with Ctrl-'-'
. The idea is to offer to the user a way to quickly insert the most used macros.
The body of the macro to insert must contain a single '|', which is a
placeholder for the selected text; the macro can also have a shortcut, or can be
selected with the mouse or by hitting the Return key. The Esc key close the
window. For example, if the user needs macro \framed as in
\framed[frame=on]{...} (where '...' means arbitrary text) and he wants the
single key 'f' as shortcut, the text to insert is
(f)\framed[frame=on]{|}

(don't forget of the enclosing '(' and ')' ).
The user's macros are stored in ConTeXt.ini, and this file can be edited from
Plugin->ConTeXt->Edit Config; once saved it has to be re-loaded with
Plugin->ConTeXt->Load Config (no need to restart Notepad++). For sake of
readability, the characters '%n' in the body of the text is translated into a
newline. The shortcut can be a sequence of A-Za-z0-9 characters, i.e. (d0) and
(sst) are valid shortcuts, but in this case perhaps one needs to set a delay
with usermacro:elapse=500 and or usermacro:elapse_shift=600 (the time is
milliseconds) under the [CommandsSetups] section of ConTeXt.ini .


A second mechanism (from the WebEdit plugin) are Tags. Under this section, there
is a list of text -> replacement: in the edit are the user type 'text' and with
Alt-'\' this is replaced by replacement. The rules are the same of the user's
macro.

Finally, the standard right-click offers the usual insert text menu.

These ways are independent, so it's possible to have several ways to insert the
same text, or slightly different texts, but it's likely that in a next release
this will be simplified.

