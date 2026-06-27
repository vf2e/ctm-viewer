cmake_minimum_required(VERSION 3.16)

find_package(Qt5 5.12 REQUIRED COMPONENTS Test Widgets)

set(CTM_TEST_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/FieldColorMap.cpp
    ${CMAKE_SOURCE_DIR}/src/FieldSimulator.cpp
    ${CMAKE_SOURCE_DIR}/src/ModelLoader.cpp
)

set(CTM_TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
    ${OPENCTM_DIR}
    ${VTK_INCLUDE_DIR}
)

set(CTM_TEST_LINK_LIBS
    Qt5::Test
    Qt5::Widgets
    openctm
    ${CTM_VIEWER_VTK_LIBS}
)

if(NOT VTK_DLLS)
    file(GLOB VTK_DLLS "${VTK_BIN_DIR}/*.dll")
endif()

if(WIN32)
    list(APPEND CTM_TEST_LINK_LIBS opengl32)
endif()

function(ctm_viewer_add_test target)
    add_executable(${target} ${target}.cpp ${CTM_TEST_CORE_SOURCES})
    target_include_directories(${target} PRIVATE ${CTM_TEST_INCLUDE_DIRS})
    target_link_directories(${target} PRIVATE ${VTK_LIB_DIR} ${OPENCTM_DIR})
    target_link_libraries(${target} PRIVATE ${CTM_TEST_LINK_LIBS})
    target_compile_definitions(${target} PRIVATE
        TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    )
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENCTM_DIR}/openctm.dll"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copy openctm.dll for ${target}"
    )
    foreach(dll ${VTK_DLLS})
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}"
                "$<TARGET_FILE_DIR:${target}>"
        )
    endforeach()

    get_target_property(_qt_core_location Qt5::Core LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)

    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:${_qt_bin_dir}"
            "PATH=path_list_prepend:${VTK_BIN_DIR}"
            "PATH=path_list_prepend:${OPENCTM_DIR}"
    )
endfunction()

ctm_viewer_add_test(test_fieldcolormap)
ctm_viewer_add_test(test_fieldsimulator)
ctm_viewer_add_test(test_modelloader)
ctm_viewer_add_test(test_apptheme)
