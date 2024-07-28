cmake_minimum_required(VERSION 3.20)

project(1337)

# LLVM configuration
execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS)
execute_process(COMMAND llvm-config --ldflags --system-libs --libs core OUTPUT_VARIABLE LLVM_LDFLAGS)

string(REPLACE "\n" " " LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REGEX REPLACE "[ ]+" " " LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REGEX REPLACE "[ ]+$" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})

string(REPLACE "\n" " " LLVM_LDFLAGS ${LLVM_LDFLAGS})
string(REGEX REPLACE "[ ]+" " " LLVM_LDFLAGS ${LLVM_LDFLAGS})
string(REGEX REPLACE "[ ]+$" "" LLVM_LDFLAGS ${LLVM_LDFLAGS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")
message("[*] LLVM CXXFLAGS: ${LLVM_CXXFLAGS}")
message("[*] LLVM LDFLAGS: ${LLVM_LDFLAGS}")

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(1337 ${SRC})
target_precompile_headers(1337 PUBLIC src/llvm.hpp)
target_link_libraries(1337 ${LLVM_LDFLAGS})
