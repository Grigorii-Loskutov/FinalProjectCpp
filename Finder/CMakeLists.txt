cmake_minimum_required(VERSION 3.20)
project(Finder)

# Путь к библиотекам Boost
set(Boost_INCLUDE_DIRS "c:/vcpkg/")  

# Пути к файлам в проекте Spyder относительно Finder
set(SPIDER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../Spider")

# Файлы для добавления из Spyder в Finder
set(SPIDER_FILES
    ${SPIDER_SOURCE_DIR}/database.h
    ${SPIDER_SOURCE_DIR}/database.cpp
    ${SPIDER_SOURCE_DIR}/ParcerINI.h
    ${SPIDER_SOURCE_DIR}/ParcerINI.cpp
)

set(PROJECT_SOURCES
    main.cpp
)

find_package(Boost 1.81.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(Finder ${PROJECT_SOURCES} ${SPIDER_FILES})

add_subdirectory(e:/Sources/libpqxx-7.7.5 libpqxx-build) # Путь к libpq++
target_compile_features(Finder PRIVATE cxx_std_17) # Включим C++17
target_link_libraries(Finder pqxx)