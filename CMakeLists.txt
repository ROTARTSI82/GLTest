cmake_minimum_required(VERSION 3.14)
project(GLTest)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

print_all_variables()

add_executable(GLTest src/main.cpp)

target_include_directories(GLTest PUBLIC include src include/imgui)
target_include_directories(GLTest PUBLIC ${OPENGL_INCLUDE_DIR})

target_link_libraries(GLTest ${OPENGL_gl_LIBRARY})

target_link_libraries(GLTest ${OPENGL_glu_LIBRARY})

#target_link_libraries(GLTest glfw)

target_link_libraries(GLTest ${PROJECT_SOURCE_DIR}/lib/libglfw.3.3.dylib ${PROJECT_SOURCE_DIR}/lib/libGLEW.2.1.0.dylib)


