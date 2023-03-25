# save-manager

An SKSE plugin that removes and optionally compresses old saves files every time the player saves.

By default every time the player saves, save files are removed until only the 10 most recent remain (based on modification time).

If compression is enabled, the saves are compressed into a zip file before being removed.

## Configuration

At the moment the configuration uses the json format. I'll switch to ini files when I find the time to look for a lib for that.

In the meantime, the configuration is stored in `save-manager.json` and looks like this :

    {
        "saves_path": "Documents/My Games/Skyrim Special Edition/Saves",
        "compress": false,
        "archive_path": "",
        "n_saves_to_keep": 10,
        "max_saves_in_archive": 1000
    }

These are the default values.

If the json file does not exist, the defaults are used.

If the json file exists but some keys are missing, the defaults are used for these keys.

If the directory `saves_path` does not exist, the plugin does nothing.

If `compress == false`, the other settings regarding saves compression are ignored.