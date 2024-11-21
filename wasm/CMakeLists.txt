cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_file(_EMSCRIPTEN_EMCC_EXECUTABLE emcc)
if(EXISTS ${_EMSCRIPTEN_EMCC_EXECUTABLE})
    get_filename_component(EMSCRIPTEN ${_EMSCRIPTEN_EMCC_EXECUTABLE} DIRECTORY)
else()
  message( FATAL_ERROR "Specify path to EMCC!" )
endif()

# Enable Optimizations
if(${OPTMZ}) 
  message("OPTIMIZING")
  set(OPTIMIZE "-O3")
  set(LINK_OPTIMIZE "-flto")
  set(DEBUG "")
  set(ASSERTIONS "")
else()
  message("NOT OPTIMIZING")
  set(OPTIMIZE "-O3")
  set(LINK_OPTIMIZE "")
  set(DEBUG "-g3")
  set(ASSERTIONS "-sASSERTIONS=1")
endif()

if (${SANITIZE})
  message("SANITIZING")
  set(TOTAL_MEMORY 1048576000 CACHE STRING "Total Memory")
  set(SAN "-fsanitize=address")
else()
  message("NO SANITIZING")
  set(TOTAL_MEMORY 16252928 CACHE STRING "Total Memory")
  set(SAN "")
endif()

set(CMAKE_C_COMPILER "${EMSCRIPTEN}/emcc")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN}/em++")
set(CMAKE_CXX_FLAGS "${DEBUG} ${SAN} ${OPTIMIZE} ${LINK_OPTIMIZE} -isystem -Weffc++ -Wover-aligned -Wc++17-extensions -Wbad-function-cast -Wcast-function-type")

set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_FIND_ROOT_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# set the project name
project(network_testbed VERSION 1.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(PYTHON python3)

# Disable bullet graphical benchmarks
set(USE_GRAPHICAL_BENCHMARK OFF)

# Disable bullet GLUT
set(USE_GLUT OFF)

# Weird shit
set(MAIN_ONLOAD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/onload.js)
set(MAIN_PRELOAD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/preload.js)

# Header file and idlfor interface between cpp and js
set(MAIN_HEADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/index.hpp)
set(MAIN_IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/index.idl)

set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# EMCC binary
set(EMCC_BIN ${EMSCRIPTEN}/emcc)

# Path to the webidl bind script
set(WEBIDL_BINDER_SCRIPT ${EMSCRIPTEN}/tools/webidl_binder.py)


set(EMCC_GLUE_ARGS
  -c
  -I${BULLET_SRC_DIR}
  -I${RAPIDJSON_SRC_DIR}
  -include${MAIN_HEADER_FILE})

# GLUE
add_custom_command(
  OUTPUT glue.cpp glue.js
  BYPRODUCTS parser.out WebIDLGrammar.pkl
  COMMAND ${PYTHON} ${WEBIDL_BINDER_SCRIPT} ${MAIN_IDL_FILE} glue
  DEPENDS ${MAIN_IDL_FILE} 
  COMMENT "Generating bindings"
  VERBATIM)
add_custom_command(
  OUTPUT glue.o
  COMMAND ${EMCC_BIN} glue.cpp ${EMCC_GLUE_ARGS} -o glue.o
  DEPENDS glue.cpp ${MAIN_HEADER_FILE}
  COMMENT "Building bindings"
  VERBATIM)
add_custom_target(bindings ALL DEPENDS glue.js glue.o)

# ADD CLIENT AND COMMON LIBRARIES
add_subdirectory(src/game game_library)
add_subdirectory(src/network network_library)
add_subdirectory(src main_library)

add_executable(index $<TARGET_OBJECTS:main_library> $<TARGET_OBJECTS:game_library> $<TARGET_OBJECTS:network_library> glue.o)
target_link_libraries(index ${CONAN_LIBS})
target_link_options(index PUBLIC 
  "SHELL:--post-js glue.js"
  "SHELL:--pre-js ${MAIN_PRELOAD_FILE}"
  "SHELL:--post-js ${MAIN_ONLOAD_FILE}"
  ${MULTI}
  ${OPTIMIZE}
  ${DEBUG}
  ${SAN}
  ${WASMW}
  ${ASSERTIONS}
  ${SAFE_HEAP}
  "SHELL:-s LLD_REPORT_UNDEFINED"
  "SHELL:-s AUTOLOAD_DYLIBS=0"
  "SHELL:-s WASM_BIGINT"
  "SHELL:-s IGNORE_CLOSURE_COMPILER_ERRORS=1"
  "SHELL:-s ALLOW_TABLE_GROWTH=1"
  "SHELL:-s EXPORTED_FUNCTIONS=['_malloc','_free']"
  "SHELL:-s EXPORTED_RUNTIME_METHODS=['UTF8ToString','cwrap','addFunction']"
  "SHELL:-s EXPORT_NAME='Main'"
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s NO_DISABLE_EXCEPTION_CATCHING=1"
  "SHELL:-s WASM=1"
  "SHELL:-s NO_EXIT_RUNTIME=1"
  "SHELL:-s NO_FILESYSTEM=1"
  "SHELL:-s TOTAL_MEMORY=${TOTAL_MEMORY}"
  "SHELL:-s BINARYEN_IGNORE_IMPLICIT_TRAPS=1"
  "SHELL:-s ALLOW_MEMORY_GROWTH=0"
)