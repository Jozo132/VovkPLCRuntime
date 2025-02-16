Import("env")

# Remove -fno-common flag
env.ProcessUnFlags("-fno-common")

# Add custom linker flags
env.Append(LINKFLAGS=[
    #"-shared",
    "-fcommon",
    #"-fPIC",

    
    #"-static",
    #"-static-libgcc",
    #"-static-libstdc++"
])

