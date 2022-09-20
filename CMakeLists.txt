# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.11)
project(mock-vhost-user)
set(CMAKE_CXX_STANDARD 17)

set(obj "./util/shm.cpp")
#set(flags "-O3 -march=native -std=c++17 -pthread -mtune=native -Wall -Wextra -ffast-math -mfpmath=both -g")
set(flags "-O3 -march=x86-64 -std=c++17 -pthread -mtune=generic -Wall -Wextra -ffast-math -mfpmath=both -g")
set(CMAKE_CXX_FLAGS "${flags}")

add_executable(rx.out rx.cpp ${obj})
target_link_libraries(rx.out PUBLIC rt)
target_include_directories(rx.out PUBLIC util)
add_executable(tx.out tx.cpp ${obj})
target_link_libraries(tx.out PUBLIC rt)
target_include_directories(tx.out PUBLIC util)
add_executable(nf.out nf.cpp ${obj})
target_link_libraries(nf.out PUBLIC rt)
target_include_directories(nf.out PUBLIC util)

add_custom_command(
	TARGET nf.out
	POST_BUILD
	COMMAND cp ../shells/run.sh ./ && cp ../shells/statrun.sh ./
)
