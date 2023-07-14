import subprocess

# Check if the system is Windows
import platform
is_windows = platform.system() == "Windows"

# The target build script is located in './test/VovkPLCRuntimeWasmTestKit'
# If the system is Windows, the target executable in the given location is called 'build.bat'
# Otherwise, it is called 'build.sh'

BUILD_PATH = "test/VovkPLCRuntimeWasmTestKit/"
BUILD_FILE = "build.bat" if is_windows else "build.sh"

print("----------------------------------------------------------------------")
print("Note: to compile the WASM executable, you need to have LLVM installed.")
print("----------------------------------------------------------------------")
print("Building the WASM executable using: " + BUILD_PATH + BUILD_FILE + " ...")
# Run the target build script
bashCommand = BUILD_PATH + BUILD_FILE
process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
output, error = process.communicate()

# Check if the build was successful
if error is not None:
    print("Error building the WASM executable.")
    print(error)
else:
    print("Done.")
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
exit(process.returncode)
