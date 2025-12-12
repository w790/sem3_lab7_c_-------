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

# Ищем Boost библиотеки
find_package(Boost 1.71.0 REQUIRED COMPONENTS program_options filesystem)

if(NOT Boost_FOUND)
    # Если не нашли в системе, пробуем установить
    message(WARNING "Boost not found, trying to find in alternative locations")

    # Для Mac с Homebrew
    if(APPLE)
        set(BOOST_PATH "/opt/homebrew")
        if(EXISTS "${BOOST_PATH}")
            set(Boost_INCLUDE_DIR "${BOOST_PATH}/include")
            set(Boost_LIBRARY_DIR "${BOOST_PATH}/lib")
            find_package(Boost 1.71.0 REQUIRED COMPONENTS program_options filesystem
                    HINTS "${BOOST_PATH}")
        endif()
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
)

if(Boost_FOUND)
    target_include_directories(bayan PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(bayan ${Boost_LIBRARIES})
else()
    # Fallback для Mac
    if(APPLE)
        set(BOOST_PATH "/opt/homebrew")
        target_include_directories(bayan PRIVATE "${BOOST_PATH}/include")
        target_link_libraries(bayan
                "${BOOST_PATH}/lib/libboost_program_options.a"
                "${BOOST_PATH}/lib/libboost_filesystem.a"
        )
    else()
        message(FATAL_ERROR "Boost libraries not found!")
    endif()
endif()

# Добавляем CPack
include(CPack)
set(CPACK_PACKAGE_VENDOR "GitHub Actions")
set(CPACK_PACKAGE_CONTACT "github-actions@example.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "GitHub Actions <github-actions@example.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Duplicate File Finder")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})