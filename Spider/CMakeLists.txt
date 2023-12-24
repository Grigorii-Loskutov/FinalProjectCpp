cmake_minimum_required(VERSION 3.20)
project(Spyder)

# Устанавливаем опцию компилятора для работы с UTF-8
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

# Установка пути к Boost
set(BOOST_ROOT "c:/vcpkg/") # Путь к папке, в которой установлен Boost

set(PROJECT_SOURCES
        ParcerINI.h
        ParcerINI.cpp
        HTTPclient.h
        HTTPclient.cpp
        ParcerHTML.h
        ParcerHTML.cpp
        database.h
        database.cpp
        main.cpp
)

# Поиск и подключение Boost
find_package(Boost 1.81.0 COMPONENTS locale REQUIRED)

add_executable(Spyder ${PROJECT_SOURCES} "database.h" "database.cpp")

# Включение директорий Boost
include_directories(${Boost_INCLUDE_DIRS})

# Связывание библиотеки Boost с проектом
target_link_libraries(Spyder ${Boost_LIBRARIES})

add_subdirectory(e:/Sources/libpqxx-7.7.5 libpqxx-build) # Путь к libpq++
target_compile_features(Spyder PRIVATE cxx_std_17) # Включение C++17
target_link_libraries(Spyder pqxx)