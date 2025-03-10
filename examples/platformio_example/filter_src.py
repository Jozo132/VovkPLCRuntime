Import("env")


def skip_from_build(node):
    """
    `node.name` - a name of File System Node
    `node.get_path()` - a relative path
    `node.get_abspath()` - an absolute path
     to ignore file from a build process, just return None
    """
    if "simulator" in node.name:
        # Return None for exclude
        return None

    if "test_" in node.name:
        # Return None for exclude
        return None
    
    return node

# Register callback
env.AddBuildMiddleware(skip_from_build, "*")