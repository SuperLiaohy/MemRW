# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/MemRW_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/MemRW_autogen.dir/ParseCache.txt"
  "MemRW_autogen"
  )
endif()
