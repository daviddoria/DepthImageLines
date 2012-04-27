cmake_minimum_required(VERSION 2.6)

Project(DepthImageLines)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_KEYWORDS")

# Submodules
add_subdirectory(PTXTools)
add_subdirectory(ScribbleInteractorStyle)

if(NOT VTKHelpers_SOURCE_DIR)
 add_subdirectory(VTKHelpers)
endif()

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(DepthImageLines_UISrcs MainWindow.ui)
QT4_WRAP_CPP(DepthImageLines_MOCSrcs MainWindow.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE( ${USE_VTK_FILE} )

FIND_PACKAGE(ITK REQUIRED)
INCLUDE( ${USE_ITK_FILE} )

ADD_EXECUTABLE(DepthImageLines main.cpp MainWindow.cpp Helpers.cpp
${DepthImageLines_MOCSrcs} ${DepthImageLines_UISrcs})
TARGET_LINK_LIBRARIES(DepthImageLines ${VTK_LIBRARIES}
PTX
libHelpers
libVTKHelpers
libScribble
QVTK ${QT_LIBRARIES}
${ITK_LIBRARIES}
)
