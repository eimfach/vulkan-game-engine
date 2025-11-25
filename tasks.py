import os
import shutil
from invoke.tasks import task
import platform
from enum import StrEnum

class Mode(StrEnum):
    RELEASE="Release"
    DEBUG="Debug"
    RELWITHDEBINFO="RelWithDebInfo"
    MINSIZEREL="MinSizeRel"

# Define paths
BUILD_DIR = os.path.abspath("build")
SOURCE_DIR = os.path.abspath(".")

@task(help = {"mode": "Sets the compile mode (CMake flags as compiler flags): Release, Debug, RelWithDebInfo, MinSizeRel"})
def config(c, mode=Mode.RELEASE, forceninja=False):
    """
    Configures the project using CMake and vcpkg.
    """
    print(f"--- Configure build in {mode} Mode.")
    mode_dir = create_mode_dir(BUILD_DIR, mode, forceninja)

    # 1. Create Build Directory
    if not os.path.exists(mode_dir):
        os.makedirs(mode_dir)

    # 2. Run CMake Configure
    # -S = Source Dir, -B = Build Dir, -G = Generator (Ninja)
    cmd = [
        "cmake",
        f"-S {SOURCE_DIR}",
        f"-B {mode_dir}",
        f"-DCMAKE_BUILD_TYPE={mode}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=1" # Generates compile_commands.json
    ]
    if platform.system() == "Linux" or forceninja:
        cmd.append("-GNinja")

    c.run(" ".join(cmd))

    # 3. Symlink compile_commands.json (for IDEs/LSPs)
    # This makes clangd or VS Code C++ tools work immediately at root level
    source_cc = os.path.join(mode_dir, "compile_commands.json")
    target_cc = os.path.join(".", "compile_commands.json")
    
    if os.path.lexists(target_cc):
        try:
            os.remove(target_cc)
        except OSError as ose:
            print("Warning: Could not remove compile_commands.json. LSP might not work as expected. \n", ose)
    
    # Symlinks on Windows often require admin privs, so we fallback to copy if link fails
    
    if platform.system() == "Windows":
        return
        
    try:
        os.symlink(source_cc, target_cc)
        print(f"Symlinked {target_cc}")
    except OSError:
        shutil.copy(source_cc, target_cc)
        print(f"Copied {target_cc} (Symlink failed or not supported)")
    except PermissionError:
        shutil.copy(source_cc, target_cc)
        print("Permission Warning: Copied {target_cc} (Symlink failed or not supported. \n")
            
@task(help = {"mode": "Compile the preconfigured build for given mode."})
def build(c, mode=Mode.RELEASE, forceninja=False):):
    """
    Builds the project.
    """
    print(f"--- Compiling build in {mode} Mode.")
    mode_dir = create_mode_dir(BUILD_DIR, mode, forceninja)
    if not os.path.exists(mode_dir):
        print(f"build directory for mode: \"{mode}\" does not exist, configure the project first (invoke config --mode [Release|Debug]).")
        return
    print(mode_dir)
    flags = ""
    if platform.system() == "Windows":
        flags = f"--config {mode}"
        
    c.run(f"cmake --build {mode_dir} {flags}")
    
    dist_path = mode_dir
    if platform.system() == "Windows":
        dist_path = os.path.join(mode_dir, mode)
    
    copy_assets(dist_path, "textures")
    copy_assets(dist_path, "shaders")
    copy_assets(dist_path, "models")

@task(help = {"mode": "Run the compiled build for given mode."})
def run(c, mode=Mode.RELEASE):
    """
    Runs the executable.
    """
    ext = ".exe" if platform.system() == "Windows" else ""
    # TODO: "app" string is static but set in CMakeLists.txt
    app_path = f"{BUILD_DIR}{os.path.sep}{mode}{os.path.sep}nEngine{ext}"
    if not os.path.exists(app_path):
        build(c)

    print(f"\n--- Running build in {mode} Mode: {app_path}\n\n")
    c.run(app_path)

@task
def clean(c, mode=Mode.RELEASE):
    """
    Deletes the contents of the build directory
    """
    c.run(f"rm -rf {BUILD_DIR}{os.path.sep}{mode}{os.path.sep}*")

def copy_assets(dist_path, folder):
    shutil.copytree(f"{SOURCE_DIR}{os.path.sep}{folder}", f"{dist_path}{os.path.sep}{folder}", dirs_exist_ok=True)
    
def create_mode_dir(build_dir, mode, forceninja=False):
    if platform.system() != "Windows" or forceninja:
        return os.path.join(build_dir, mode)
    
    return build_dir