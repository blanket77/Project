# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Tutorial3_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Tutorial3_autogen.dir\\ParseCache.txt"
  "Tutorial3_autogen"
  )
endif()
