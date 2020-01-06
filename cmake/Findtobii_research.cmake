# Copyright Chadwick Boulay and Tristan Stenner
# Distributed under the MIT License.
# See https://opensource.org/licenses/MIT

set(TOBIIPRO_ROOT_DIR "${TOBIIPRO_ROOT_DIR}"
	CACHE PATH "The root to search for TobiiPro SDK"
)

# find the TOBIIPROsdk.h header in the TOBIIPRO_ROOT_DIR/include
find_path(TOBIIPRO_INCLUDE_DIR
    NAME "tobii_research.h"
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/tobii ${TOBIIPRO_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(TOBIIPRO_LIBRARY
    NAME tobii_research
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/tobii ${TOBIIPRO_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tobii_research
	REQUIRED_VARS TOBIIPRO_INCLUDE_DIR TOBIIPRO_LIBRARY)
if(tobii_research_FOUND)
    if(NOT TARGET TobiiPro::tobii_research)
        add_library(TobiiPro::tobii_research SHARED IMPORTED)

        # On Windows, the IMPORTED_LIB is the .lib, and the IMPORTED_LOCATION (below) is the .dll
        get_filename_component(libext ${TOBIIPRO_LIBRARY} EXT)
        if(libext STREQUAL ".lib")
            set_target_properties(TobiiPro::tobii_research PROPERTIES
                IMPORTED_IMPLIB ${TOBIIPRO_LIBRARY})
            string(REPLACE ".lib" ".dll" TOBIIPRO_LIBRARY ${TOBIIPRO_LIBRARY})
        endif()

        set_target_properties(TobiiPro::tobii_research PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${TOBIIPRO_INCLUDE_DIR}
            IMPORTED_LOCATION ${TOBIIPRO_LIBRARY}
            IMPORTED_LINK_INTERFACE_LANGUAGES C
            INTERFACE_COMPILE_FEATURES cxx_std_14
        )
    endif()
endif()
mark_as_advanced(TOBIIPRO_ROOT_DIR TOBIIPRO_INCLUDE_DIR TOBIIPRO_LIBRARY)
