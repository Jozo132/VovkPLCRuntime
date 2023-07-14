import subprocess
from os.path import abspath
import platform

is_windows = platform.system() == "Windows"

BUILD_PATH = "./test/VovkPLCRuntimeWasmTestKit/"
BUILD_FILE = "build.bat" if is_windows else "build.sh"
TARGET_RELATIVE = BUILD_PATH + BUILD_FILE
TARGET = abspath(TARGET_RELATIVE)

print("----------------------------------------------------------------------")
print("Note: to compile the WASM executable, you need to have LLVM installed.")
print("----------------------------------------------------------------------")
print("Building the WASM executable ...")
print("Executing: " + TARGET_RELATIVE)
stderr = subprocess.call([TARGET], shell=True)

# Get the error code
code = stderr

# Check if the build was successful
if code:
    print("Error building the WASM executable.")
    print(code)
else:
    msg = "The compiled WASM file is located in: ./" + BUILD_PATH + "simulator.wasm"
    print("-" * len(msg))
    print(msg)
    print("Along with the JS wrapper: ./" + BUILD_PATH + "simulator.js")
    print("-" * len(msg))
    print("To use it, navigate into the " + BUILD_PATH + " directory")
    print("and you can either test it like for example:")
    print("   - Node.JS -> cd into 'node-test' and run 'node run.js'")
    print("   - Browser -> cd into 'web-server-test' and run 'node server.js'")
    print("Or you can copy the 'simulator.wasm' and 'simulator.js' files into your project and use them there as you wish.")

# Exit with the same code as the build script
exit(code)
