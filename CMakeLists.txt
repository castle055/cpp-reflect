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

cmake_minimum_required(VERSION 4.3 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_BMI_DIRECTORY "${CMAKE_BINARY_DIR}/bmi")
add_compile_options(
        -fimplicit-constexpr
        -freflection
        -fconstexpr-ops-limit=100000000
        -fconstexpr-depth=2048
)

project(cpp_reflect LANGUAGES CXX)

SET(CPP_REFLECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CPP_REFLECT_TEST_DIR ${CPP_REFLECT_DIR}/test)
SET(CPP_REFLECT_INCLUDE_DIR ${CPP_REFLECT_DIR}/include)
SET(CPP_REFLECT_SOURCE_DIR ${CPP_REFLECT_DIR}/src)


################################################################################
#[[                          CONFIGURE DEPENDENCIES                          ]]#
################################################################################
include(FetchContent)

################################################################################
#[[                         CONFIGURE CLANG PLUGINS                          ]]#
#add_subdirectory(clang)

#if(CPP_REFLECT_PLUGIN_BUILD)
#    return()
#endif()

#if(NOT EXISTS ${REFLECTOR_PLUGIN_BINARY})
#    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
#            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
#            -DCMAKE_BUILD_TYPE=Release #${CMAKE_BUILD_TYPE}
#            -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
#            -DCPP_REFLECT_PLUGIN_BUILD=ON
#            ${CMAKE_CURRENT_SOURCE_DIR}
#            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target ReflectorPlugin
#            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#endif()
#
#macro(compile_target_with_reflection TARGET)
#    set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-fplugin=${REFLECTOR_PLUGIN_BINARY}")
#endmacro(compile_target_with_reflection)

#[[                            CONFIGURE PACKTL                              ]]#
FetchContent_Declare(packtl
        GIT_REPOSITORY https://github.com/castle055/packtl.git
        GIT_TAG main
        FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(packtl)
set_target_properties(packtl PROPERTIES FOLDER "Deps")

#[[                         CONFIGURE GOOGLE TESTS                           ]]#
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest PROPERTIES FOLDER "Deps")
set_target_properties(gtest_main PROPERTIES FOLDER "Deps")
set_target_properties(gmock PROPERTIES FOLDER "Deps")
set_target_properties(gmock_main PROPERTIES FOLDER "Deps")


################################################################################
#[[                          CONFIGURE CPP REFLECT                           ]]#
add_library(cpp_reflect)
#compile_target_with_reflection(cpp_reflect)

#macro(reflect_target TARGET)
#    compile_target_with_reflection(${TARGET})
#    target_link_libraries(${TARGET} PUBLIC cpp_reflect)
#endmacro(reflect_target)

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
SET(SRC_MOD_LIST
        include/reflection/descriptors/FundamentalInfo.cppm
        include/reflection/descriptors/FieldInfo.cppm
        include/reflection/descriptors/ClassInfo.cppm
        include/reflection/descriptors/TypeInfo.cppm
        include/reflection/descriptors/PointerInfo.cppm
        include/reflection/descriptors/EnumInfo.cppm
        include/reflection/descriptors/FunctionInfo.cppm
        include/reflection/descriptors/descriptors.cppm
        include/reflection/generation/Reflector.cppm
)

target_include_directories(cpp_reflect PUBLIC ${CPP_REFLECT_SOURCE_DIR})
target_include_directories(cpp_reflect PUBLIC ${cyd_fabric_SOURCE_DIR}/include)
target_compile_features(cpp_reflect PUBLIC cxx_std_26)

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
if (CPP_REFLECT_DEV_MODE AND CPP_REFLECT_BUILD_TESTS)
    include(GoogleTest)
    get_filename_component(TEST_DIR ${CPP_REFLECT_TEST_DIR} REALPATH)

    FILE(GLOB_RECURSE TEST_LIST
            ${TEST_DIR}/**/*.cpp
            ${TEST_DIR}/*.cpp
    )

    add_executable(cpp_reflect_tests ${TEST_LIST})
    target_link_libraries(cpp_reflect_tests PRIVATE cpp_reflect gtest gtest_main)

    gtest_discover_tests(cpp_reflect_tests)
endif ()


################################################################################
#[[                     CONFIGURE DOXYGEN DOCUMENTATION                      ]]#
################################################################################
if (CPP_REFLECT_DEV_MODE AND CPP_REFLECT_BUILD_DOCS)
    target_configure_doxygen(cpp_reflect
            DOXYFILE ${CPP_REFLECT_DIR}/docs/Doxyfile
            BASE_DIR ${CPP_REFLECT_DIR}
    )
endif ()
