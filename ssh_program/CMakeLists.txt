cmake_minimum_required(VERSION 3.10)
project(ssh_program)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED libssh)

add_executable(ssh_program main.cpp)

target_include_directories(ssh_program PRIVATE ${LIBSSH_INCLUDE_DIRS})
target_link_libraries(ssh_program PRIVATE ${LIBSSH_LIBRARIES})