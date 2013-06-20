# Required project namespacing because CMake uses it to set a whole
# host of auto-magic variables that are convenient for larger
# projects. Typically these are in the form <PROJECT-NAME>_<VAR-NAME>.
#
# Note that there is also PROJECT_NAME which will store the value sent
# to project().
#
PROJECT(tabularasa)

# Required version check, or CMake will throw a tantrum.
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# GCC Include directories for -I 
#
# It gives access to the user level file which is in
# include/tabularasa.h and the kernel level header
# include/linux/tabularasa.h
#
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )


# Flags that CMAKE will as arguments to GCC that are not source files,
# includes, and libraries. For each 
#
SET(CMAKE_C_FLAGS "-Wall")


# Each add_subdirectory call specifies a subdirectory that contains
# its own CMake file that pertains to building the stuff in that
# particular subdirectory.
#
# The libtabularasa directory is the user level interface to the
# module and the userprog directory is the user program using the
# library.
#
ADD_SUBDIRECTORY(libtabularasa)
ADD_SUBDIRECTORY(userprog)
