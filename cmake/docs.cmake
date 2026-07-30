# Copyright (c) 2024, Víctor Castillo Agüero.
# SPDX-License-Identifier: GPL-3.0-or-later

function(target_configure_doxygen TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 "DOCS" "" "DOXYFILE;BASE_DIR" "")
    if (NOT DOCS_BASE_DIR)
        message(FATAL_ERROR "target_configure_doxygen() requires BASE_DIR to be provided as an argument")
    endif ()
    if (NOT DOCS_DOXYFILE)
        message(FATAL_ERROR "target_configure_doxygen() requires DOXYFILE to be provided as an argument")
    endif ()

    add_custom_target(
            ${TARGET}_docs
            COMMAND /usr/bin/doxygen ${DOCS_DOXYFILE}
            WORKING_DIRECTORY ${DOCS_BASE_DIR}
            COMMENT "Generating DOXYGEN documentation..."
    )
    add_custom_command(TARGET ${TARGET}_docs PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory docs
            WORKING_DIRECTORY ${DOCS_BASE_DIR}
    )
endfunction()