cmake_minimum_required(VERSION 3.8)
project(TobiiPro
	LANGUAGES CXX
	VERSION 0.1.0)
include(Findliblsl.cmake)

# create an imported target we later add the required information to
add_library(tobii SHARED IMPORTED)

# find the tobii_research.h header in the subfolder tobii/include
find_path(TOBII_SDK_INCLUDE_PATH
	name "tobii_research.h"
	PATHS "tobii/include"
)
# find a shared library called tobii_research.dll or libtobii_research.so
# in tobii/lib
find_library(TOBII_SDK_LIB tobii_research
	PATHS "${CMAKE_CURRENT_LIST_DIR}/tobii/lib"
)
if(NOT TOBII_SDK_LIB)
	message(FATAL_ERROR "Tobii SDK library not found")
endif()
# Windows needs additional help
get_filename_component(libext ${TOBII_SDK_LIB} EXT)
if(libext STREQUAL ".lib")
	set_target_properties(tobii PROPERTIES IMPORTED_IMPLIB ${TOBII_SDK_LIB})
	string(REPLACE ".lib" ".dll" TOBII_SDK_LIB ${TOBII_SDK_LIB})
endif()
# add the paths we just found to the tobii library target
set_target_properties(tobii PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${TOBII_SDK_INCLUDE_PATH}"
	IMPORTED_LOCATION "${TOBII_SDK_LIB}"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
	TobiiLib.h
	tobiilogo.qrc
)

add_executable(tobiisynctest tobiisynctest.cpp)
target_link_libraries(tobiisynctest tobii)
add_executable(tobiifinder tobiifinder.cpp)
target_link_libraries(tobiifinder tobii)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Widgets
	Threads::Threads
	LSL::lsl
	tobii
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
target_compile_features(tobiisynctest PUBLIC cxx_std_14)

installLSLApp(${PROJECT_NAME})
installLSLAuxFiles(${PROJECT_NAME}
	${PROJECT_NAME}.cfg
	tobiilogo.svg
	${TOBII_SDK_LIB}
)
LSLGenerateCPackConfig()
