
# Setting the CMake module path to specify the 
# directory into which CMake should look to 
# find extra modules, I.E. FindLibXml2.cmake
#
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Since libxml2 is a little strange we have to the super special
# finder library (./cmake/FindLibXml2.cmake). This will set a few
# variables (defined in the source file) that makes our life a little
# easier. These variables are in the form LIBXML2_<NAME>. Also it may
# be useful to test the result variable LIBXML2_FOUND, but GCC will
# yell at us anyway.
include(FindLibXml2)

# GCC Include directories for -I
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/xhashconf
  ${LIBXML2_INCLUDE_DIRS}
  )


# Flags that CMAKE will pass to GCC not related to source files,
# includes, and libraries.
#
set(CMAKE_C_FLAGS "-Wall ${LIBXML2_DEFINITIONS}")


# Create an executable target with the same name as the projects from
# the files that follow it.
ADD_EXECUTABLE(otuser 
  otuser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/config.c 
  ${CMAKE_CURRENT_SOURCE_DIR}/xhashconf/xhashconf.c )


# Libs for GCC -l
TARGET_LINK_LIBRARIES(otuser 
  occamstimer
  ${LIBXML2_LIBRARIES}
  )





# Make a list of files from the globbing expressions relative to the
# source directory and store that list in CONFIG_FILES.
#
# file(GLOB CONFIG_FILES 
#   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#   *cpu/*.dski
#   *cpu/*.pipes
#   *cpu/*.hgs
#   *cpu/*.xml
#   )

  
# # Copy all of the configuration files into the binary directory.
# #
# foreach(CF ${CONFIG_FILES})

#   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${CF} 
#     ${CMAKE_CURRENT_BINARY_DIR}/${CF} COPYONLY)

# endforeach(CF)
