cmake_minimum_required(VERSION 2.8)

# Project Name
PROJECT(HW_OPENGL)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLM) 
find_package(PHYSX)

find_path(GLM_INCLUDE_DIR glm/glm.hpp) 

set(PHYSX_INCLUDE_DIRS 
/home/kevin/Libraries/PhysXSDK_3.3/Include/
/home/kevin/Libraries/PhysXSDK_3.3/Include/foundation/
)
set(PHYSX_LIBRARY_DIRS 
/home/kevin/Libraries/PhysXSDK_3.3/Lib/linux64/
)

include_directories( ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${PHYSX_INCLUDE_DIRS} )

add_executable(test main.cxx)

target_link_libraries(test ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARY} ${PHYSX_LIBRARY_DIRS} )
 
