cmake_minimum_required(VERSION 3.8)
project(TobiiPro
	LANGUAGES CXX
	VERSION 1.13.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(tobii_research REQUIRED)

# Find an installed liblsl in paths set by the user (LSL_INSTALL_ROOT)
# and some default paths
find_package(LSL REQUIRED
	HINTS ${LSL_INSTALL_ROOT}
	"${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/"
	"${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/out/build/x64-Release"
	PATH_SUFFIXES share/LSL)
get_filename_component(LSL_PATH ${LSL_CONFIG} DIRECTORY)
message(STATUS "Found LSL lib in ${LSL_PATH}")
LSLAPP_Setup_Boilerplate()

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
target_link_libraries(tobiisynctest TobiiPro::tobii_research)
add_executable(tobiifinder tobiifinder.cpp)
target_link_libraries(tobiifinder TobiiPro::tobii_research)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Widgets
	Threads::Threads
	LSL::lsl
	TobiiPro::tobii_research
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
