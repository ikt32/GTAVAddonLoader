Requirements
============
GTA V (v1.0.877.1 or later)
ScriptHookV

Installation
============
Put AddonSpawner.asi and the AddonSpawner folder in the main GTA V folder.

Usage
=====
Press F5 to show the menu, or enter the "addonspawner" cheat.

Add image previews:
Put a png or jpg image inside AddonSpawner/img, with the file name being the
spawn name of the vehicle. Recommended: png, 480x270. Any other aspect ratio
also works. Try keeping the file small for better performance.

User DLC Instructions
=====================
Create a `<DLC Name>.list` file with the model names you want in the DLC.
For example, if you have a few vanilla vehicles, create the following file:

VanillaWorks Lite.list

Inside the `.list` file, put each model you want on a new line, for example

streiter2
vincent2

A `VanillaWorks Lite` entry will then appear in the main menu - with under it,
the vehicles you added. Still sortable by brand or class, giving you more power
to manage your add-on vehicle packs.


Changing hotkey
===============
Change MenuKey in settings_menu.ini to something else. Available keys are
in Keyboard_Keys.txt

You can also assign a controller key combo to open the menu. 
settings_menu.ini should contain all info you need.

Language packs
==============
Language packs are UTF-8 XML files in AddonSpawner/Languages. Change the
language from the in-game Settings menu. Included languages are English and
Simplified Chinese. Chinese menu text and subtitles have been verified in
GTA V Legacy v1.0.3788.0. Missing translations automatically fall back to
English. Additional languages can be added after native-speaker translation
or review.

To add a language, copy en-US.xml, change the code and name on the <language>
element, and translate the values inside each <text> element. Do not change the
text keys or named placeholders such as {vehicle}, {model}, {count}, {dlc},
{version}, {category}, and {value}. Entries with missing or extra placeholders
fall back to English. XML reserved characters must be escaped,
for example &amp; for an ampersand and &quot; for a quote. Language files are
rescanned whenever the menu opens.

Source
======
https://github.com/E66666666/GTAVAddonLoader

