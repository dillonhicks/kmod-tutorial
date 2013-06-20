# Required project namespacing because CMake uses it to set a whole
# host of auto-magic variables that are convenient for larger
# projects. Typically these are in the form <PROJECT-NAME>_<VAR-NAME>.
#
# Note that there is also PROJECT_NAME which will store the value sent
# to project().
#
PROJECT(occamstimer)

# Required version check, or CMake will throw a tantrum.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(BLANKSLATE_DIR $ENV{PWD})

# GCC Include directories for -I
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )


# Flags that CMAKE will pass to GCC not related to source files,
# includes, and libraries.
#
SET(CMAKE_C_FLAGS "-Wall")

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(userprog)
