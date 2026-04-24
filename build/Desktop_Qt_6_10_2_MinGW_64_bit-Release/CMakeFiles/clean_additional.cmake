# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\QtBCI_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\QtBCI_autogen.dir\\ParseCache.txt"
  "QtBCI_autogen"
  )
endif()
