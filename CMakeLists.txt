cmake_minimum_required(VERSION 3.10)
project(otus7 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_USE_STATIC_LIBS ON)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-unused-variable)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O0")

# REQUIRED PACKAGES
find_package(Boost COMPONENTS unit_test_framework program_options filesystem regex system REQUIRED)
find_package(OpenSSL REQUIRED)


# LIBRARIES
add_library(hash hash.cpp hash.h)
set_target_properties(hash PROPERTIES
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)
target_link_libraries(hash ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

add_library(reader reader.cpp reader.h)
set_target_properties(reader PROPERTIES
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)
target_link_libraries(reader ${Boost_LIBRARIES})

add_library(file_filter file_filter.cpp file_filter.h)
set_target_properties(file_filter PROPERTIES
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)
target_link_libraries(file_filter ${Boost_LIBRARIES})

add_library(scanner scanner.cpp scanner.h)
set_target_properties(scanner PROPERTIES
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)
target_link_libraries(scanner ${Boost_LIBRARIES} hash reader file_filter)


# EXECUTABLE
add_executable(otus7 main.cpp)
set_target_properties(otus7 PROPERTIES
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)
target_link_libraries(otus7 ${Boost_LIBRARIES} scanner)

#include_directories(${OPENSSL_INCLUDE_DIR})

# TESTS
add_executable(test_scanner test_scanner.cpp)
set_target_properties(test_scanner PROPERTIES
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
target_link_libraries(test_scanner scanner ${Boost_LIBRARIES})

add_executable(test_file_filter test_file_filter.cpp)
set_target_properties(test_file_filter PROPERTIES
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
target_link_libraries(test_file_filter file_filter ${Boost_LIBRARIES})

add_executable(test_hash test_hash.cpp)
set_target_properties(test_hash PROPERTIES
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
target_link_libraries(test_hash hash ${Boost_LIBRARIES})

add_executable(test_reader test_reader.cpp)
set_target_properties(test_reader PROPERTIES
        INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
target_link_libraries(test_reader reader ${Boost_LIBRARIES})

enable_testing()
add_test(test_scanner test_scanner)
add_test(test_file_filter test_file_filter)
add_test(test_hash test_hash)
add_test(test_reader test_reader)

install(TARGETS otus7 RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEB_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${otus7_VERSION})
set(CPACK_PACKAGE_CONTACT buevichd@yandex-team.ru)
include(CPack)

