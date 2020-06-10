cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(GSTPrintNvDSMetadata CXX)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_NAME GSTPrintNvDSMetadata)

set(DEEPSTREAM_VERSION 5.0)
set(CUDA_VER 10.2)

find_package(PkgConfig REQUIRED)
find_package(CUDA REQUIRED)

MESSAGE(STATUS "CUDA found ${CUDA_FOUND}")
message(STATUS "CUDA LIBS: ${CUDA_LIBRARIES}")
message(STATUS "CUDA INCLUDES: ${CUDA_INCLUDE_DIRS}")

pkg_check_modules(GSTREAMER "gstreamer-1.0" REQUIRED)
message(STATUS "GSTREAMER LIBS: ${GSTREAMER_LIBRARIES}")
message(STATUS "GSTREAMER INCLUDES: ${GSTREAMER_INCLUDE_DIRS}")
message(STATUS "GSTREAMER Compile Options: ${GSTREAMER_CFLAGS_OTHER}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "BuildType: ${CMAKE_BUILD_TYPE}")

include_directories(.)
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories("${CUDA_INCLUDE_DIRS}")

link_directories("/opt/nvidia/deepstream/deepstream-${DEEPSTREAM_VERSION}/lib")
link_directories("/usr/local/cuda-$(CUDA_VER)/lib64/")

file (GLOB GST_PLZ_PRINT_META_SOURCES "plzprintmeta.cpp")
file (GLOB GST_PLZ_PRINT_META_HEADERS "plzprintmeta.h")

add_library(gstplzprintmeta SHARED "${GST_PLZ_PRINT_META_SOURCES}")
target_link_libraries(gstplzprintmeta PUBLIC "${GSTREAMER_LIBRARIES}" "${CUDA_LIBRARIES}" "-lnvdsgst_meta" "-lnvds_meta" "-lgstbase-1.0" "-lnvds_inferutils" "-lnvds_utils")
target_include_directories(gstplzprintmeta PUBLIC "${GSTREAMER_INCLUDE_DIRS}" "/opt/nvidia/deepstream/deepstream-${DEEPSTREAM_VERSION}/sources/includes" "/usr/local/cuda-$(CUDA_VER)/include")
target_compile_options(gstplzprintmeta PUBLIC "${GSTREAMER_CFLAGS_OTHER}" )

EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )

INSTALL(TARGETS gstplzprintmeta EXPORT ${PROJECT_NAME}Config
        ARCHIVE DESTINATION /usr/lib/${ARCHITECTURE}-linux-gnu/gstreamer-1.0/
        LIBRARY DESTINATION /usr/lib/${ARCHITECTURE}-linux-gnu/gstreamer-1.0/
)
