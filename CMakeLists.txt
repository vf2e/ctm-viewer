cmake_minimum_required(VERSION 3.16)
project(ctm-viewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(MAGIMAIM_QT_ROOT "D:/2025WorkSpace/magaim-qt" CACHE PATH
    "Path to magaim-qt project (provides VTK64 and openctm)")

set(VTK_ROOT "${MAGIMAIM_QT_ROOT}/3dLib/VTK64" CACHE PATH "VTK 8.0 root")
set(VTK_VERSION "8.0")
set(OPENCTM_DIR "${MAGIMAIM_QT_ROOT}/3dLib/openctm" CACHE PATH "OpenCTM library directory")

set(VTK_INCLUDE_DIR "${VTK_ROOT}/include/vtk-${VTK_VERSION}")
set(VTK_LIB_DIR "${VTK_ROOT}/lib")
set(VTK_BIN_DIR "${VTK_ROOT}/bin")

if(NOT EXISTS "${VTK_INCLUDE_DIR}/vtkPolyData.h")
    message(FATAL_ERROR "VTK headers not found at ${VTK_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${OPENCTM_DIR}/openctm.lib")
    message(FATAL_ERROR "openctm.lib not found at ${OPENCTM_DIR}")
endif()

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/VtkRenderWidget.cpp
    src/ModelLoader.cpp
    src/FieldColorMap.cpp
    src/FieldSimulator.cpp
    src/ColorBarWidget.cpp
    src/FloatPanel.cpp
    src/SettingsDialog.cpp
)

set(HEADERS
    src/MainWindow.h
    src/VtkRenderWidget.h
    src/ModelLoader.h
    src/FieldColorMap.h
    src/FieldSimulator.h
    src/ColorBarWidget.h
    src/FloatPanel.h
    src/SettingsDialog.h
    src/AppTheme.h
)

set(RESOURCES
    resources/app.qrc
)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES} ${APP_ICON_RESOURCE_WINDOWS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OPENCTM_DIR}
    ${VTK_INCLUDE_DIR}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${VTK_LIB_DIR}
    ${OPENCTM_DIR}
)

# 从 magaim-qt 精简的 VTK 8.0 模块
set(CTM_VIEWER_VTK_LIBS
    vtkRenderingOpenGL-${VTK_VERSION}
    vtkRenderingCore-${VTK_VERSION}
    vtkRenderingFreeType-${VTK_VERSION}
    vtkRenderingContextOpenGL-${VTK_VERSION}
    vtkRenderingContext2D-${VTK_VERSION}
    vtkRenderingAnnotation-${VTK_VERSION}
    vtkInteractionStyle-${VTK_VERSION}
    vtkIOGeometry-${VTK_VERSION}
    vtkIOCore-${VTK_VERSION}
    vtkIOLegacy-${VTK_VERSION}
    vtkFiltersCore-${VTK_VERSION}
    vtkFiltersGeneral-${VTK_VERSION}
    vtkFiltersGeometry-${VTK_VERSION}
    vtkFiltersSources-${VTK_VERSION}
    vtkImagingCore-${VTK_VERSION}
    vtkCommonExecutionModel-${VTK_VERSION}
    vtkCommonDataModel-${VTK_VERSION}
    vtkCommonCore-${VTK_VERSION}
    vtkCommonColor-${VTK_VERSION}
    vtkCommonMath-${VTK_VERSION}
    vtkCommonTransforms-${VTK_VERSION}
    vtkCommonSystem-${VTK_VERSION}
    vtkCommonMisc-${VTK_VERSION}
    vtksys-${VTK_VERSION}
    vtkzlib-${VTK_VERSION}
    vtkfreetype-${VTK_VERSION}
    vtkpng-${VTK_VERSION}
    vtkjpeg-${VTK_VERSION}
    vtktiff-${VTK_VERSION}
    vtkexpat-${VTK_VERSION}
    vtkjsoncpp-${VTK_VERSION}
    vtkmetaio-${VTK_VERSION}
    vtkverdict-${VTK_VERSION}
    vtklibxml2-${VTK_VERSION}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    openctm
    ${CTM_VIEWER_VTK_LIBS}
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copy assets directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENCTM_DIR}/openctm.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copy openctm.dll"
)

file(GLOB VTK_DLLS "${VTK_BIN_DIR}/*.dll")
foreach(dll ${VTK_DLLS})
    get_filename_component(dll_name ${dll} NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copy ${dll_name}"
    )
endforeach()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

option(BUILD_TESTING "Build and register unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
