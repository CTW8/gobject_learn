cmake_minimum_required(VERSION 3.10)
project(gobjectDemo VERSION 0.1.0 LANGUAGES C)

# 查找 pkg-config
find_package(PkgConfig REQUIRED)

# 查找 glib 和 gobject
pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0)

# 使用 find_library 查找 bzip2 库
find_library(BZIP2_LIBRARY bz2 PATHS /opt/homebrew/opt/bzip2/lib NO_DEFAULT_PATH)

# 检查是否找到 bzip2
if(NOT BZIP2_LIBRARY)
    message(FATAL_ERROR "未找到 bzip2 库。请确保 bzip2 已安装并且路径正确。")
endif()

# 查找 zlib
find_package(ZLIB REQUIRED)

# 查找 iconv（使用 find_library 而非 find_package）
find_library(ICONV_LIBRARY iconv REQUIRED)

# 查找 CoreFoundation 和 Security 框架（仅适用于 macOS）
if(APPLE)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    find_library(SECURITY_FRAMEWORK Security REQUIRED)
endif()

# 查找 SDL2（假设 SDL2 不是 keg-only）
find_library(SDL2_LIBRARY SDL2 PATHS /opt/homebrew/opt/sdl2/lib NO_DEFAULT_PATH)
if(NOT SDL2_LIBRARY)
    message(FATAL_ERROR "未找到 SDL2 库。请确保 SDL2 已安装并且路径正确。")
endif()

# 打印找到的库信息（调试用）
message(STATUS "BZip2_LIBRARY: ${BZIP2_LIBRARY}")
message(STATUS "SDL2_LIBRARY: ${SDL2_LIBRARY}")

# 查找所有源文件
file(GLOB_RECURSE SOURCES "src/**/*.c")

# 添加可执行文件
add_executable(gobjectDemo main.c ${SOURCES})

# 包含头文件路径（使用 target_include_directories 代替全局 include_directories）
target_include_directories(gobjectDemo PRIVATE 
    ${GLIB_INCLUDE_DIRS}
    /opt/homebrew/opt/bzip2/include      # 手动添加 bzip2 的包含路径
    ${ZLIB_INCLUDE_DIRS}
    /opt/homebrew/opt/sdl2/include       # 手动添加 SDL2 的包含路径
    ${CMAKE_SOURCE_DIR}/lib/libffmpeg/include
    ${CMAKE_SOURCE_DIR}/src/test1
    ${CMAKE_SOURCE_DIR}/src/test2
)

# 添加编译选项（使用 target_compile_options 代替全局 add_definitions）
target_compile_options(gobjectDemo PRIVATE ${GLIB_CFLAGS_OTHER})

# 指定链接库目录
target_link_directories(gobjectDemo PRIVATE 
    ${CMAKE_SOURCE_DIR}/lib/libffmpeg/lib
    ${GLIB_LIBRARY_DIRS}
    /opt/homebrew/opt/bzip2/lib          # 手动添加 bzip2 的库目录
    /opt/homebrew/opt/sdl2/lib           # 手动添加 SDL2 的库目录
    ${ZLIB_LIBRARY_DIRS}
)

# 链接库，确保 bzip2 和 SDL2 在 FFmpeg 之后
target_link_libraries(gobjectDemo PRIVATE 
    ${GLIB_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARY}                      # 链接 bzip2 库
    ${ICONV_LIBRARY}
    ${SDL2_LIBRARY}                       # 链接 SDL2 库
    avcodec avformat avutil swresample swscale avfilter
)

# 仅在 macOS 上链接 CoreFoundation 和 Security 框架
if(APPLE)
    target_link_libraries(gobjectDemo PRIVATE 
        ${COREFOUNDATION_FRAMEWORK}
        ${SECURITY_FRAMEWORK}
    )
endif()