# Copyright (c) 2024, Víctor Castillo Agüero.
# SPDX-License-Identifier: GPL-3.0-or-later


################################################################################
#[[                            PROJECT OVERVIEW                              ]]#
################################################################################
#                                                               CPP-REFLECT    #
#                                                                              #
#[[  AVAILABLE OPTIONS:                                                      ]]#
#      - CPP_REFLECT_BUILD_TESTS ........................ DEV_MODE only, ON    #
#            whether or not tests should be built                              #
#      - CPP_REFLECT_BUILD_DOCS ......................... DEV_MODE only, ON    #
#            whether or not the documentation should be built                  #
#                                                                              #
#[[  CMAKE STRUCTURE:                                                        ]]#
#      - Project setup                                                         #
#      - Configure dependencies                                                #
#      - Configure CPP-REFLECT                                                 #
#      - Configure tests                                                       #
#      - Configure Doxygen documentation                                       #
#                                                                              #
################################################################################


################################################################################
#[[                              PROJECT SETUP                               ]]#
################################################################################
#[[                          PREVENT IN-TREE BUILD                           ]]#
if (CMAKE_SOURCE_DIR STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build. Create a `build/` directory and call `cmake ..` from there.")
endif ()

#[[                          CHECK DEVELOPER MODE                            ]]#
set(CPP_REFLECT_DEV_MODE OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CPP_REFLECT_DEV_MODE ON)
endif ()

#[[                         DEVELOPER MODE OPTIONS                           ]]#
if(CPP_REFLECT_DEV_MODE)
    option(CPP_REFLECT_BUILD_TESTS "whether or not tests should be built" ON)
    option(CPP_REFLECT_BUILD_DOCS "whether or not the documentation should be built" ON)
endif ()

# Select 'Release' build type by default.
# Has to be done before the call to `project()`.
# Use `-DCMAKE_BUILD_TYPE=` to override this.
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

cmake_minimum_required(VERSION 3.29.20240518 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -pthread")

project(cpp_reflect LANGUAGES CXX)

SET(CPP_REFLECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CPP_REFLECT_TEST_DIR ${CPP_REFLECT_DIR}/test)
SET(CPP_REFLECT_INCLUDE_DIR ${CPP_REFLECT_DIR}/include)
SET(CPP_REFLECT_SOURCE_DIR ${CPP_REFLECT_DIR}/src)
SET(CPP_REFLECT_APPS_DIR ${CPP_REFLECT_DIR}/apps)


################################################################################
#[[                          CONFIGURE DEPENDENCIES                          ]]#
################################################################################
include(FetchContent)

#[[                            CONFIGURE PACKTL                              ]]#
FetchContent_Declare(packtl
        GIT_REPOSITORY https://github.com/castle055/packtl.git
        GIT_TAG main
        FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(packtl)

################################################################################
#[[                         CONFIGURE CLANG PLUGINS                          ]]#
add_subdirectory(clang)

if(CPP_REFLECT_PLUGIN_BUILD)
    return()
endif()

if(NOT EXISTS ${REFLECTOR_PLUGIN_BINARY})
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
            -DCPP_REFLECT_PLUGIN_BUILD=ON
            ${CMAKE_SOURCE_DIR}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target ReflectorPlugin
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

macro(compile_target_with_reflection TARGET)
    set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-fplugin=${REFLECTOR_PLUGIN_BINARY}")
endmacro(compile_target_with_reflection)

################################################################################
#[[                          CONFIGURE CPP REFLECT                           ]]#
add_library(cpp_reflect)
compile_target_with_reflection(cpp_reflect)

macro(reflect_target TARGET)
    compile_target_with_reflection(${TARGET})
    target_link_libraries(${TARGET} PUBLIC cpp_reflect)
endmacro(reflect_target)

FILE(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS
        ${CPP_REFLECT_INCLUDE_DIR}/*.h
        ${CPP_REFLECT_INCLUDE_DIR}/*.hpp
        ${CPP_REFLECT_INCLUDE_DIR}/*.inc
        ${CPP_REFLECT_SOURCE_DIR}/*.h
        ${CPP_REFLECT_SOURCE_DIR}/*.hpp
        ${CPP_REFLECT_SOURCE_DIR}/*.inc
        ${CPP_REFLECT_SOURCE_DIR}/*.cpp
)
FILE(GLOB_RECURSE SRC_MOD_LIST CONFIGURE_DEPENDS
        ${CPP_REFLECT_INCLUDE_DIR}/*.cppm
        ${CPP_REFLECT_SOURCE_DIR}/*.cppm
)
FILE(GLOB_RECURSE SRC_MODI_LIST CONFIGURE_DEPENDS
        ${CPP_REFLECT_INCLUDE_DIR}/*.cppi
        ${CPP_REFLECT_SOURCE_DIR}/*.cppi
)

target_include_directories(cpp_reflect PUBLIC ${CPP_REFLECT_SOURCE_DIR})
target_include_directories(cpp_reflect PUBLIC ${cyd_fabric_SOURCE_DIR}/include)
target_compile_features(cpp_reflect PUBLIC cxx_std_23)

target_sources(cpp_reflect
        PUBLIC ${SRC_LIST}
        PUBLIC ${SRC_MODI_LIST}
        PUBLIC
        FILE_SET cxx_modules
        #        FILE_SET "${APP_NAME}_modfiles"
        TYPE CXX_MODULES
        BASE_DIRS ${CPP_REFLECT_SOURCE_DIR} ${CPP_REFLECT_INCLUDE_DIR}
        FILES ${SRC_MOD_LIST}
)

target_link_libraries(cpp_reflect PUBLIC packtl)


################################################################################
#[[                             CONFIGURE TESTS                              ]]#
################################################################################
enable_testing()
if (CPP_REFLECT_DEV_MODE AND CPP_REFLECT_BUILD_TESTS)
    target_configure_test_directory(cpp_reflect ${CPP_REFLECT_TEST_DIR} TEST_TARGET_LIST)

    foreach(TARGET_NAME ${TEST_TARGET_LIST})
        message("FOUND TARGET: " ${TARGET_NAME})
        if("${TARGET_NAME}" MATCHES "^TEST_.*")
            target_link_libraries(${TARGET_NAME})
            reflect_target(${TARGET_NAME})
        endif()
    endforeach()
endif ()


################################################################################
#[[                     CONFIGURE DOXYGEN DOCUMENTATION                      ]]#
################################################################################
if (CPP_REFLECT_DEV_MODE AND CPP_REFLECT_BUILD_DOCS)
    target_configure_doxygen(cpp_reflect
            DOXYFILE ${CPP_REFLECT_DIR}/Doxyfile
            BASE_DIR ${CPP_REFLECT_DIR}
    )
endif ()
