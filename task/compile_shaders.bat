glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\simple_shader.vert -o .\shaders\simple_shader.vert.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\simple_shader.frag -o .\shaders\simple_shader.frag.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\point_light.vert -o .\shaders\point_light.vert.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\point_light.frag -o .\shaders\point_light.frag.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\line.vert -o .\shaders\line.vert.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\line.frag -o .\shaders\line.frag.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\aabb.vert -o .\shaders\aabb.vert.spv
glslc.exe --target-env=vulkan1.0 --target-spv=spv1.0 .\shaders\aabb.frag -o .\shaders\aabb.frag.spv

REM spirv-dis.exe ..\shaders\simple_shader.vert.spv
REM spirv-dis.exe ..\shaders\simple_shader.frag.spv
