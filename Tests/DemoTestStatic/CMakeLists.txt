cmake_minimum_required(VERSION 2.8)

# Project Name
PROJECT(HW_OPENGL)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLM) 

find_path(GLM_INCLUDE_DIR glm/glm.hpp) 

set(TINY_OBJ_DIR 3rdParty/tinyObjLoader)

include_directories( ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${TINY_OBJ_DIR})

add_executable(test main.cpp 3rdParty/tinyObjLoader/tiny_obj_loader.cc)

target_link_libraries(test ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARY} )
 
