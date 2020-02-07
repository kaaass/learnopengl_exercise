cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME learnopengl)
project(${PROJECT_NAME})
aux_source_directory(. DIR_SRCS)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} ${DIR_SRCS})

# OpenGl staffs
set(OPENGL_INC_DIR
    /usr/local/include
    /usr/include
    ${PROJECT_SOURCE_DIR}/include)
set(OPENGL_LIBS GLEW glfw GL X11 pthread Xrandr Xi)
include_directories(${OPENGL_INC_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBS})