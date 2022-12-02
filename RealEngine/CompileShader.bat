SET workdir=%~dp0

 %workdir%Vulkan/Bin/glslc.exe Shader/shader.vert -o Shader/vert.spv
 %workdir%Vulkan/Bin/glslc.exe Shader/shader.frag -o Shader/frag.spv