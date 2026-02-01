Presets (SQLite)
===============

What is a preset?
-----------------
A preset is just the plugin's parameter state saved to disk.

Web-dev analogy:
- Preset = saved JSON object
- SQLite = a small embedded database (like a local file-backed store)

Why SQLite instead of JSON files?
---------------------------------
- Fast search and sorting
- Easy to add tags/metadata later
- Single file (no folder of scattered JSONs)

How we store presets
--------------------
We serialize the JUCE parameter state to a binary blob and store it in SQLite.

Table schema (auto-created)
---------------------------
```
presets(
  name TEXT PRIMARY KEY,
  data BLOB NOT NULL,
  updated_at INTEGER NOT NULL
)
```

API in this boilerplate
-----------------------
- `open(dbPath)`
- `savePreset(name, blob)`
- `loadPreset(name, outBlob)`
- `listPresets()`
- `deletePreset(name)`

Typical flow
------------
1) `exportPresetBlob()` from the processor.
2) Save it with `PresetStore::savePreset()`.
3) Later, load it and call `importPresetBlob()`.

UI in this boilerplate
----------------------
- A preset name field + Save button
- A preset list dropdown + Load / Delete buttons
- The list refreshes after save/delete
