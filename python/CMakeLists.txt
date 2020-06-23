cmake_minimum_required (VERSION 3.0)

project(PyVkInline)

add_custom_target(Run_CFFIBuild
COMMAND python VkInline/cffi_build.py
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running cffi_build")

set (SRC
api.h
api_utils.cpp
api_Context.cpp
api_ShaderViewable.cpp
api_SVBuffer.cpp
api_SVCombine.cpp
api_SVObjBuffer.cpp
api_Texture2D.cpp
api_Texture3D.cpp
)

set (INCLUDE_DIR 
.
..
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
add_definitions(${DEFINES})

add_library(PyVkInline SHARED ${SRC})
target_link_libraries(PyVkInline VkInline)


if (WIN32) 
install(TARGETS PyVkInline RUNTIME DESTINATION test_python/VkInline)
else()
install(TARGETS PyVkInline DESTINATION test_python/VkInline)
endif()


set(PYTHON
	VkInline/__init__.py
	VkInline/cffi.py
	VkInline/Native.py
	VkInline/utils.py
	VkInline/Context.py	
	VkInline/ShaderViewable.py
	VkInline/SVBuffer.py	
	VkInline/SVCombine.py
	VkInline/SVObjBuffer.py
	VkInline/SVVector.py
	VkInline/SVObjVector.py
	VkInline/Texture2D.py
	VkInline/Texture3D.py
)

install(FILES ${PYTHON} DESTINATION test_python/VkInline)
install(FILES setup.py DESTINATION test_python)

set(VKINLINE_INCLUDE_PYTESTS false CACHE BOOL "Include tests")

if (VKINLINE_INCLUDE_PYTESTS)
add_subdirectory(test)
endif()

