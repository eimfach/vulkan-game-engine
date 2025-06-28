glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 ..\shaders\simple_shader.vert -o ..\shaders\simple_shader.vert.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 ..\shaders\simple_shader.frag -o ..\shaders\simple_shader.frag.spv
spirv-dis.exe ..\shaders\simple_shader.vert.spv
spirv-dis.exe ..\shaders\simple_shader.frag.spv
pause