cmake_minimum_required(VERSION 2.6)

Project(DepthImageLines)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_KEYWORDS")

add_subdirectory(PTXTools)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(UISrcs MainWindow.ui)
QT4_WRAP_CPP(MOCSrcs MainWindow.h)

include_directories(${include_directories} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE( ${USE_VTK_FILE} )

FIND_PACKAGE(ITK REQUIRED)
INCLUDE( ${USE_ITK_FILE} )

ADD_EXECUTABLE(DepthImageLines main.cpp MainWindow.cpp vtkScribbleInteractorStyle.cxx Helpers.cpp
${MOCSrcs} ${UISrcs})
TARGET_LINK_LIBRARIES(DepthImageLines ${VTK_LIBRARIES}
PTX
libHelpers
libVTKHelpers
QVTK ${QT_LIBRARIES}
${ITK_LIBRARIES}
)
