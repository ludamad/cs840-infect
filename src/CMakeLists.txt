cmake_minimum_required(VERSION 2.6)
project(infectsim)

find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)

add_subdirectory(libs/UnitTest++)

include_directories(
	libs/
    libs/UnitTest++
    "libs/mersenne-simd"
)

add_definitions(
    -std=c++11 ${EXTRA_DEFS}
)
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

aux_source_directory("." infectsim_src) 
aux_source_directory("libs" infectsim_src) 
aux_source_directory("libs/mersenne-simd" infectsim_src) 
add_executable(infectsim ${infectsim_src})

target_link_libraries(infectsim UnitTest++ ${SDL_LIBRARY} ${SDL_TTF_LIBRARY})

