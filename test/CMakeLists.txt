cmake_minimum_required (VERSION 3.0)

add_executable(test_vki test.cpp)

target_link_libraries(test_vki VkInline)

install(TARGETS test_vki RUNTIME DESTINATION test_cpp)

