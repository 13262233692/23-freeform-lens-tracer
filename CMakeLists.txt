cmake_minimum_required(VERSION 3.16)
project(FreeformLensTracer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/GLCanvas.cpp
    src/NURBS/NURBSSurface.cpp
    src/NURBS/NURBSTessellator.cpp
    src/Renderer/Camera.cpp
    src/Renderer/GLRenderer.cpp
    src/RayTracer/OpticalMath.cpp
    src/RayTracer/RayTracerEngine.cpp
)

set(HEADERS
    src/MainWindow.h
    src/GLCanvas.h
    src/NURBS/NURBSSurface.h
    src/NURBS/NURBSTessellator.h
    src/Renderer/Camera.h
    src/Renderer/GLRenderer.h
    src/RayTracer/OpticalMath.h
    src/RayTracer/RayTracerEngine.h
)

set(SHADERS
    src/shaders/surface.vert
    src/shaders/surface.frag
    src/shaders/ray.vert
    src/shaders/ray.frag
    src/shaders/light.vert
    src/shaders/light.frag
)

qt_add_resources(RESOURCES resources.qrc)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NURBS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RayTracer
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
)
