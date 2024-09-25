cmake_minimum_required(VERSION 3.10)
project(gobjectDemo VERSION 0.1.0 LANGUAGES C)

# 查找 pkg-config
find_package(PkgConfig REQUIRED)

# 查找 glib 和 gobject
pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0)

# 包含头文件路径
include_directories(${GLIB_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/src/test1
                    ${CMAKE_SOURCE_DIR}/src/test2)

# 添加编译选项
add_definitions(${GLIB_CFLAGS_OTHER})

file(GLOB_RECURSE SOURCES "src/**/*.c")

add_executable(gobjectDemo main.c ${SOURCES})


target_link_directories(gobjectDemo PUBLIC 
                                ${GLIB_LIBRARY_DIRS})
# 链接库
target_link_libraries(gobjectDemo PRIVATE ${GLIB_LIBRARIES})