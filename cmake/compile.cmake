add_compile_options(-Wall -Wextra -pedantic -Werror -std=c++1z -fprofile-arcs -ftest-coverage -g)
set (CMAKE_SHARED_LINKER_FLAGS "-fprofile-arcs --coverage")
