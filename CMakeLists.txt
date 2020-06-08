cmake_minimum_required (VERSION 3.0)

set (SKIP_GLSLANG_INSTALL true)
set (SPIRV_SKIP_TESTS true)
set (SPIRV_CROSS_ENABLE_TESTS false)
set (SPIRV_CROSS_SKIP_INSTALL true)
set (SPIRV_CROSS_STATIC true)

project(VkInline)

set (INCLUDE_DIR
thirdparty
thirdparty/volk
thirdparty/glslang
thirdparty/SPIRV-Cross
thirdparty/Vulkan-Headers/include
thirdparty/crc64
thirdparty/unqlite
thirdparty/Vulkan_utils
.
./internal
)


if (WIN32) 
set (DEFINES  ${DEFINES}
-D"_CRT_SECURE_NO_DEPRECATE"  
-D"_SCL_SECURE_NO_DEPRECATE" 
-D"_CRT_SECURE_NO_WARNINGS"
)
else()
add_definitions(-std=c++17)
add_compile_options(-fPIC)
endif()


include_directories(${INCLUDE_DIR})
add_subdirectory(thirdparty/volk)
add_subdirectory(thirdparty/glslang)
add_subdirectory(thirdparty/SPIRV-Cross)

set (LIB_SOURCES
thirdparty/crc64/crc64.cpp
thirdparty/Vulkan_utils/vk_format_utils.cpp
internal/internal_context.cpp
glslc.cpp
Context.cpp
SVBuffer.cpp
SVCombine.cpp
SVObjBuffer.cpp
)

set (LIB_HEADERS
ShaderViewable.h
Context.h
SVBuffer.h
SVCombine.h
SVObjBuffer.h
)


set(INTERNAL_HEADERS
thirdparty/crc64/crc64.h
internal/internal_context.h
internal/impl_context.inl
)

add_definitions(${DEFINES})
add_library(unqlite STATIC thirdparty/unqlite/unqlite.h thirdparty/unqlite/unqlite.c)
add_library(VkInline ${LIB_SOURCES} ${LIB_HEADERS} ${INTERNAL_HEADERS})
target_link_libraries(VkInline volk glslang SPIRV spirv-cross-glsl unqlite)


if (WIN32) 
else()
target_link_libraries(VkInline dl)
endif()


IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX  ../install CACHE PATH "Install path" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

install(FILES ${LIB_HEADERS} DESTINATION include)
install(TARGETS VkInline DESTINATION lib)

set(VKINLINE_BUILD_TESTS false CACHE BOOL "Build tests")

if (VKINLINE_BUILD_TESTS)
add_subdirectory(test)
endif()

set(BUILD_PYTHON_BINDINGS true CACHE BOOL "Build Python Bindings")

if (BUILD_PYTHON_BINDINGS)
add_subdirectory(python)
endif()
