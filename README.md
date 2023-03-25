# save-manager

An SKSE plugin that removes and optionally compresses old saves files every time the player saves.

By default save files are removed until only the 10 most recent remain (based on modification time).

If compression is enabled, the saves are compressed into a zip file before being removed.

## Why

I'm always scared that Todd corrupts my save so I only create new saves instead of overwriting to be able to go back in time if some troublesome bug occurs. This quickly leads to a bloated save directory holding hundreds of files.

I use to manually manage them but I grew tired of it so I did this plugin to do it automatically.

## Configuration

At the moment the configuration uses the json format. I'll switch to ini files when I find the time to look for a lib for that.

In the meantime, the configuration is stored in `save-manager.json` and looks like this :

    {
        "saves_path": "Documents/My Games/Skyrim Special Edition/Saves",
        "compress": false,
        "archive_path": "Documents/My Games/Skyrim Special Edition/save-manager.zip",
        "n_saves_to_keep": 10,
        "max_saves_in_archive": 1000
    }

These are the default values.

If the json file does not exist, the defaults are used.

If the json file exists but some keys are missing, the defaults are used for these keys.

If the directory `saves_path` does not exist, the plugin does nothing.

If `compress == false`, the other settings regarding saves compression are ignored.