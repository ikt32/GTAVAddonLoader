# DLC (User DLC / Game DLC)

The loader supports two kinds of user-defined DLC folders, both living under
the mod folder (`<mod folder>\AddonSpawner`), which let you group add-on
vehicles into named entries that show up in the menu just like official DLCs.

Both use the exact same `.list` file format described below - the only
difference is *what* they expand and how entries are ordered.

| Folder     | Purpose                                                                   | Ordering                                     |
|------------|----------------------------------------------------------------------------|-----------------------------------------------|
| `UserDLC`  | Groups your own add-on vehicle packs under a custom name.                  | Unordered, listed under "User DLC".            |
| `GameDLC`  | Expands the hard-coded, built-in official DLC list (see `VehicleHashes.h` / `buildDLClist()`) to temporarily cover vehicles from a new official update the loader doesn't know about yet. | Always placed after all built-in DLCs, sorted by an ordering key embedded in the filename. |

`GameDLC` entries are automatically and silently ignored once an official
script update adds the same DLC natively (matched by name, case-insensitive),
so there's no need to remove outdated `GameDLC` files yourself.

## File format

Each DLC definition is a plain text file with the extension `.list`: one
vehicle model name per line, e.g.:

```
outlaw
outlaw2
tulip
tulip2
```

## Filename convention

### UserDLC

Filenames are simply `<Name>.list`, e.g. `VanillaWorks Lite.list`. The
filename (minus extension) becomes the DLC name shown in the menu.

### GameDLC

Filenames must follow the pattern:

```
<key>_<Name>.list
```

- `<key>` is only used to order this GameDLC entry relative to other GameDLC
  entries. Use a sortable value such as a date, e.g. `2024-06`.
- `<Name>` is the display name of the DLC, shown in menus exactly like the
  built-in DLC names.

Example: `2024-06_Bottom Dollar Bounties.list`

If the filename has no underscore, the entire filename (minus extension) is
used as the Name, and the entry sorts before any keyed entries.
