cmake_minimum_required(VERSION 3.20)

project(nEngineProject CXX)

# --------------------------------------------------------
# 1. SETUP SOURCES
# --------------------------------------------------------
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "src/*.h"
)

# --------------------------------------------------------
# 2. SETUP DEPENDENCIES
# --------------------------------------------------------

# --- A. GLFW (The clean way) ---
# GLFW has its own CMakeLists.txt. We just include it.
# This creates the 'glfw' target automatically.
# Make sure to disable GLFW examples/docs to save build time.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory("external/glfw-3.4") 


# --- B. ImGui (The manual way) ---
# ImGui is just source files. We must explicitly list them.
# We include the Core files AND the Backends for glfw and vulkan.
add_library(imgui STATIC
    "external/imgui-1.92.1/imgui.cpp"
    "external/imgui-1.92.1/imgui_demo.cpp"
    "external/imgui-1.92.1/imgui_draw.cpp"
    "external/imgui-1.92.1/imgui_tables.cpp"
    "external/imgui-1.92.1/imgui_widgets.cpp"
    # Backends
    "external/imgui-1.92.1/backends/imgui_impl_glfw.cpp"
    "external/imgui-1.92.1/backends/imgui_impl_vulkan.cpp"
)

# Allow ImGui headers to be found by anyone linking to 'imgui'
target_include_directories(imgui PUBLIC 
    "external/imgui-1.92.1" 
    "external/imgui-1.92.1/backends"
    # ImGui backends need access to GLFW headers to compile!
    "external/glfw-3.4/include" 
)

# --- C. Vulkan (The standard way) ---
# Don't hardcode paths to $ENV{VULKAN_SDK}. Let CMake find it.
find_package(Vulkan REQUIRED)

# --- D. fastgltf ---
# fastgltf has its own CMakeLists.txt. We just include it.
# This creates the 'fastgltf' target automatically.
set(FASTGLTF_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(FASTGLTF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FASTGLTF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FASTGLTF_COMPILE_AS_CPP20 ON)
add_subdirectory("external/fastgltf-0.9.0")

# --------------------------------------------------------
# 3. SETUP EXECUTABLE
# --------------------------------------------------------
add_executable(nEngine ${PROJECT_SOURCES})
set_property(TARGET nEngine PROPERTY CXX_STANDARD 20)

# --------------------------------------------------------
# 4. INCLUDES
# --------------------------------------------------------
target_include_directories(nEngine PRIVATE src)

# Includes for header-only libs (GLM, TinyGLTF)
target_include_directories(nEngine PRIVATE "external/glm")
target_include_directories(nEngine PRIVATE "external/tinygltf2")
target_include_directories(nEngine PRIVATE "external/fastgltf/include")
target_include_directories(nEngine PRIVATE "external/tinyobjloader")

# Note: We don't need to add GLFW or ImGui includes here manually
# because we link against them below, and they expose their includes (PUBLIC).

# --------------------------------------------------------
# 5. LINKING
# --------------------------------------------------------
target_link_libraries(imgui PRIVATE 
    Vulkan::Vulkan 
    glfw 
)

target_link_libraries(nEngine PRIVATE 
    imgui           # Our manually defined library
    glfw            # The target defined by add_subdirectory(glfw...)
    Vulkan::Vulkan  # The target defined by find_package
	fastgltf::fastgltf
)

# --------------------------------------------------------
# 6. COMPILER OPTIONS
# --------------------------------------------------------
if(MSVC)
    target_compile_options(nEngine PRIVATE /W4) # /W0 disables warnings, use /W4 for dev
else()
    target_compile_options(nEngine PRIVATE -Wall -Wextra)
endif()