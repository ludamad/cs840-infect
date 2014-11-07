cmake_minimum_required(VERSION 2.6)
project(infectsim)

find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)

add_subdirectory(libs/UnitTest++)

include_directories(
    libs/UnitTest++
)

add_definitions(
	-std=c++11
)

aux_source_directory("." infectsim_src) 
aux_source_directory("libs" infectsim_src) 
aux_source_directory("hashkat-utils" infectsim_src) 
aux_source_directory("hashkat-utils/mersenne-simd" infectsim_src) 
add_executable(infectsim ${infectsim_src})

target_link_libraries(infectsim UnitTest++ ${SDL_LIBRARY} ${SDL_TTF_LIBRARY})
