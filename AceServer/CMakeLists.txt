# CMakeList.txt : CMake project for AceServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

project ("AceServer")

add_executable (AceServer "src/AceServer.cpp")

add_compile_definitions(SRV)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(AceServer PRIVATE -Wformat=0)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AceServer PROPERTY CXX_STANDARD 20)
endif()