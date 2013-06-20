PROJECT(truser)

# Create an executable target with the same name as the projects from
# the files that follow it.
ADD_EXECUTABLE(${PROJECT_NAME}
  ${PROJECT_NAME}.c 
  )


# Libs for GCC -l
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  tabularasa
  )

