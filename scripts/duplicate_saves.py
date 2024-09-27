import argparse
import os
from pathlib import Path
import shutil

argparser = argparse.ArgumentParser()
argparser.add_argument("path", help="Path to the saves to duplicate")
argparser.add_argument("-n", "--number", help="Number of saves to create", default=100)

args = argparser.parse_args()

saves = list[Path]()

for f in os.listdir(args.path):
    filepath = Path(os.path.join(args.path, f))
    if filepath.is_file() and filepath.suffix in [".ess", ".skse"]:
        saves.append(filepath)

latest_save_index = 0
latest_save_name_rest = str()
latest_save_path = Path()
latest_skse_path = Path()

for save in saves:
    filename = save.name
    if not filename.startswith("Save"):
        continue
    index = int(filename[4:6])
    if index > latest_save_index:
        latest_save_index = index
        if filename.endswith("ess"):
            latest_save_name_rest = save.stem[6:]
            latest_save_path = save
            latest_skse_path = save.with_suffix(".skse")

print(f"Last index = {latest_save_index}")
print(f"Last save {latest_save_path} ; {latest_skse_path}")
print(f"Last save name rest = {latest_save_name_rest}")

for i in range(latest_save_index + 1, latest_save_index + args.number + 1):
    dest_stem = os.path.join(args.path, f"Save{i}{latest_save_name_rest}")
    dest_save = dest_stem + ".ess"
    dest_skse = dest_stem + ".skse"

    print(f"Copying last save to {dest_save} ; {dest_skse}")

    shutil.copyfile(latest_save_path, dest_save)
    shutil.copyfile(latest_skse_path, dest_skse)
