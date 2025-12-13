cmake_minimum_required(VERSION 3.14)
project(sem3_lab7_c--)

# Устанавливаем версию проекта
if(DEFINED ENV{PROJECT_VERSION_MAJOR})
    set(PROJECT_VERSION_MAJOR $ENV{PROJECT_VERSION_MAJOR})
    set(PROJECT_VERSION_MINOR $ENV{PROJECT_VERSION_MINOR})
    set(PROJECT_VERSION_PATCH $ENV{PROJECT_VERSION_PATCH})
else()
    set(PROJECT_VERSION_MAJOR 1)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 0)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Отключаем устаревший поиск Boost (CMP0167)
cmake_policy(SET CMP0167 NEW)

# Разные настройки для разных платформ
if(APPLE)
    # Для Mac с Homebrew
    set(BOOST_PATH "/opt/homebrew")
    if(EXISTS "${BOOST_PATH}")
        set(Boost_USE_STATIC_LIBS ON)
        set(Boost_NO_SYSTEM_PATHS ON)
        set(BOOST_ROOT "${BOOST_PATH}")
        set(BOOST_INCLUDEDIR "${BOOST_PATH}/include")
        set(BOOST_LIBRARYDIR "${BOOST_PATH}/lib")
    endif()

    find_package(Boost 1.71.0 REQUIRED COMPONENTS program_options filesystem)

elseif(WIN32)
    # Для Windows
    set(Boost_USE_STATIC_LIBS OFF)

    # Если есть переменные окружения от vcpkg
    if(DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
        set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
    endif()

    find_package(Boost 1.71.0 REQUIRED COMPONENTS program_options filesystem OPTIONAL_COMPONENTS crc)

else()
    # Для Linux
    set(Boost_USE_STATIC_LIBS OFF)

    # На Linux используем MODULE mode для FindBoost
    find_package(Boost 1.71.0 REQUIRED
            COMPONENTS program_options filesystem
            OPTIONAL_COMPONENTS crc  # crc может быть не найден как библиотека
    )

    # Проверяем наличие заголовка crc
    find_path(BOOST_CRC_INCLUDE_DIR boost/crc.hpp
            PATHS /usr/include /usr/local/include
    )

    if(NOT BOOST_CRC_INCLUDE_DIR)
        message(WARNING "boost/crc.hpp not found. The crc library may not work.")
    endif()
endif()

# Основная программа
add_executable(bayan
        main.cpp
        Hash.cpp
        ScannerDirectory.cpp
)

# Include директории
target_include_directories(bayan PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

# Линковка Boost
if(Boost_FOUND)
    target_link_libraries(bayan ${Boost_LIBRARIES})

    # Для Linux, если crc не найден как библиотека, линкуем только заголовочную версию
    if(NOT TARGET Boost::crc AND NOT "crc" IN_LIST Boost_FIND_COMPONENTS)
        message(STATUS "Boost::crc library not found, using header-only version")
    endif()
else()
    message(FATAL_ERROR "Boost libraries not found!")
endif()

# Для отладки
message(STATUS "Boost found: ${Boost_FOUND}")
message(STATUS "Boost components: ${Boost_FIND_COMPONENTS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")

# Добавляем CPack
include(CPack)
set(CPACK_PACKAGE_VENDOR "GitHub Actions")
set(CPACK_PACKAGE_CONTACT "github-actions@example.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "GitHub Actions <github-actions@example.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Duplicate File Finder")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Настройки для разных платформ
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "DEB")
endif()