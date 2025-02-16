import subprocess
from os.path import abspath
import platform

is_windows = platform.system() == "Windows"

BUILD_PATH = "./wasm/"
BUILD_FILE = "test.bat" if is_windows else "test.sh"
TARGET_RELATIVE = BUILD_PATH + BUILD_FILE
TARGET = abspath(TARGET_RELATIVE)

print("-----------------------------------------------------------------")
print("Note: to compile the WASM tests, you need to have LLVM installed.")
print("-----------------------------------------------------------------")
print("Executing: " + TARGET_RELATIVE)
stderr = subprocess.call([TARGET], shell=True)

# Get the error code
code = stderr

# Check if the build was successful
if code:
    print("Error building the WASM executable.")
    print(code)

# Exit with the same code as the build script
exit(code)
