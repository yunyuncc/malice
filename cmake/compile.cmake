add_compile_options(-Wall -Wextra -pedantic -Werror -std=c++1z)
#find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
#if (CMAKE_CXX_CPPCHECK)
#    set(CPPCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/CppCheckSuppressions.txt")
#    #set(CMAKE_CXX_CPPCHECK "cppcheck --enable=warning --inconclusive --force --inline-suppr --suppressions-list=${CPPCHECK_SUPPRESSIONS_FILE}")
#    list(
#        APPEND CMAKE_CXX_CPPCHECK 
#            "--enable=warning"
#            "--inconclusive"
#            "--force" 
#            "--inline-suppr"
#    )
#    if(EXISTS ${CPPCHECK_SUPPRESSIONS_FILE})
#        list(APPEND CMAKE_CXX_CPPCHECK "--suppressions-list=${CPPCHECK_SUPPRESSIONS_FILE}")
#    endif()
#endif()
