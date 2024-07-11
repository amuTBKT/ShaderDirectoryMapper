# ShaderDirectoryMapper
Unreal Engine plugin for automatically registering plugin and project shaders.

# Motivation
In order to use custom shaders, the engine expects them to be located in a directory virtually mapped with the shader compiler.<br>
While it's easy to add virtual shader directories, it can be a bit tedious.
* Requires code compilation so doesn't work with Content only plugins.
* At minimum it requires to add `Plugin.build.cs`, `PluginModule.cpp` and module dependencies.<br>
And this setup has to be repeated for every plugin that uses custom shaders!

# How it Works
The plugin hooks into module loading phase complete event and checks all enabled plugins and project for `/Shaders` directory.
If it finds any relevant directory and it's not already registered by the plugin itself, it adds virtual mapping for it.
* Plugins paths are registered with `/Plugin/{PluginName}` prefix.
* Game/project path is registered with `/Game` prefix.

***Note*** The shader directory name should be "Shaders"

### Example
For a plugin named "TestPlugin" and directory structure
```
TestPlugin
    -> Shaders
        -> Test.ush
```
The virtual path would be "/Plugin/TestPlugin" and header include path would be "/Plugin/TestPlugin/Test.ush"


For a project named "MyProject" and directory structure
```
MyProject
    -> Shaders
        -> Test.ush
```
The virtual path would be "/Game" and header include path would be "/Game/Test.ush"

# Settings
The plugin settings can be configured from `ProjectSettings->Plugins->Shader Directory Mapper` page, which allows to disable certain plugins or only register specified plugins.

# Contributions
Special thanks to [Victor Careil](https://x.com/phyronnaz) for suggesting virtual shader path prefixes.
