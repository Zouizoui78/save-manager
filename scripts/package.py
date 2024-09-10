import os
import shutil
import subprocess
from pathlib import Path

project_root = Path(__file__).parent.parent
os.chdir(project_root)

BUILD_PATH = Path("build/release/ae/Release")
DIST_ROOT = Path("dist/save-manager")
DIST_PLUGINS = Path(DIST_ROOT / "SKSE/Plugins")

shutil.rmtree(BUILD_PATH, ignore_errors=True)
shutil.rmtree("dist", ignore_errors=True)
subprocess.check_call("cmake --preset=release-ae")
subprocess.check_call("cmake --build build --preset=release-ae")

os.makedirs(DIST_PLUGINS)


def copyfile(src: Path, dest: Path):
    shutil.copyfile(src, dest / src.name)


copyfile(BUILD_PATH / "save-manager.dll", DIST_PLUGINS)
copyfile(Path("save-manager.json"), DIST_PLUGINS)

shutil.make_archive("dist/save-manager", "zip", DIST_ROOT)
