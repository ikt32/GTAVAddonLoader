# GTAVAddonLoader

Script that automatically detects and can load add-on vehicles, instead of needing
to type the names yourself.

## Downloads

[GTA5-Mods.com](https://www.gta5-mods.com/scripts/add-on-vehicle-spawner)

## Building

### Requirements
* [ScriptHookV SDK by Alexander Blade](http://www.dev-c.com/gtav/scripthookv/)
* [GTAVMenuBase](https://github.com/E66666666/GTAVMenuBase)

Download the [ScriptHookV SDK](http://www.dev-c.com/gtav/scripthookv/) and extract its contents to ScriptHookV_SDK.

Clone this repository to the same folder ScriptHookV_SDK was extracted so you have ScriptHookV_SDK and GTAVManualTransmission in the same folder. If you get build errors about missing functions, update your [natives.h](hhttps://raw.githubusercontent.com/E66666666/GTAVMenuBase/master/thirdparty/scripthookv-sdk-updates/natives.h).

Clone my [GTAVMenuBase](https://github.com/E66666666/GTAVMenuBase) to the same folder you're gonna clone this to.

## Language packs

Language packs are UTF-8 XML files stored in `AddonSpawner/Languages`. The
language can be changed immediately from the in-game Settings menu and is saved
as `Language` in `settings_general.ini`. The bundled packs are English
(`en-US`) and Simplified Chinese (`zh-CN`). Chinese menu text and subtitles have
been verified in GTA V Legacy v1.0.3788.0. If a pack or an individual entry
cannot be loaded, the affected text falls back to built-in English. Additional
languages can be added after native-speaker translation or review.

Create another language by copying `en-US.xml`, then change the language code,
display name and translated values:

```xml
<?xml version="1.0" encoding="utf-8"?>
<language code="fr-FR" name="Français">
  <text key="menu.main.title">Générateur de véhicules add-on</text>
  <text key="menu.main.settings">Paramètres</text>
</language>
```

Language codes are case-sensitive and should use BCP 47 form. Keep every
`key` unchanged. Unknown keys are ignored, missing or empty entries use the
English fallback, and duplicate keys in one file use the last value. Use XML
entities such as `&amp;`, `&lt;`, `&gt;`, `&quot;`, and `&apos;` for reserved
characters. Dynamic messages contain named placeholders such as `{vehicle}`,
`{model}`, `{count}`, `{dlc}`, `{version}`, `{category}`, and `{value}`; keep
the required placeholders in translated values. Entries with missing or extra
placeholders are rejected and use the English fallback.

Language files are rescanned whenever the menu opens, so new or edited packs
can be used without rebuilding the plugin or restarting the game. If multiple
files declare the same language code, the first valid file in filename order is
used (the bundled `en-US` pack customizes the built-in English fallback).
