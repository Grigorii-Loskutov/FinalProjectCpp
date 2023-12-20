cmake_minimum_required(VERSION 3.20)
project(Diplom_CPP)

#set(Boost_INCLUDE_DIRS "e:/Sources/boost_1_84_0/boost")
set(Boost_INCLUDE_DIRS "c:/vcpkg/")

set(PROJECT_SOURCES
        ParcerINI.h
        ParcerINI.cpp
        HTTPclient.h
        HTTPclient.cpp
        HTTPserverSync.h
        HTTPserverSync.cpp
        #HTTPserverSmallAsync.cpp
        #HTTPserverSmallAsync.h
        main.cpp
)

find_package(Boost 1.81.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(Diplom_CPP ${PROJECT_SOURCES})

add_subdirectory(e:/Sources/libpqxx-7.7.5 libpqxx-build) # Путь к libpq++
target_compile_features(Diplom_CPP PRIVATE cxx_std_17) # Включим C++17
target_link_libraries(Diplom_CPP pqxx)
