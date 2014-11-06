cmake_minimum_required(VERSION 2.6)
project(infectsim)

add_subdirectory(libs/UnitTest++)

include_directories(
    libs/UnitTest++
)

aux_source_directory("." infectsim_src) 
aux_source_directory("hashkat-utils" infectsim_src) 
aux_source_directory("hashkat-utils/mersenne-simd" infectsim_src) 
add_executable(infectsim ${infectsim_src})

target_link_libraries(infectsim UnitTest++)
