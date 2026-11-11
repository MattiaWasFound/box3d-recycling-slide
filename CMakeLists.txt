cmake_minimum_required(VERSION 3.22)
project(box3d_recycling_slide C)

include(FetchContent)
set(BOX3D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX3D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX3D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX3D_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	box3d
	GIT_REPOSITORY https://github.com/erincatto/box3d.git
	GIT_TAG 5d6364d7313e185e85e74674649f8e6c2ce50939
)
FetchContent_MakeAvailable(box3d)

add_executable(repro main.c)
target_link_libraries(repro PRIVATE box3d)
