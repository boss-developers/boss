cmake_minimum_required(VERSION 2.6)
project(boss)

option(BUILD_GUI "Build the BOSS GUI" ON)
option(ENABLE_ALL_WARNINGS "Enable all compilation warnings" OFF)
if (MSVC)
	if (CMAKE_CL_64)
		set(ARCH "64")
	else ()
		set(ARCH "32")
	endif ()
endif ()
#option(BUILD_SHARED_LIBS "Build Shared Library (OFF for Static)" OFF)
set (LIBGIT2_ROOT "../libgit2")
#if (MINGW)
#	list(APPEND CMAKE_MODULE_PATH "${LIBGIT2_ROOT}/cmake/Modules/")
#endif ()
#include(FindHTTP_Parser)
#set (CMAKE_VERBOSE_MAKEFILE ON)

# Main Header Files

set(BOSS_H      src/base/fstream.h
                src/base/regex.h
                src/common/conditional_data.h
                src/common/dll_def.h
                src/common/error.h
                src/common/game.h
                src/common/globals.h
                src/common/item_list.h
                src/common/keywords.h
                src/common/rule_line.h
                src/common/settings.h
                src/output/boss_log.h
                src/output/output.h
                src/parsing/grammar.h
                src/support/helpers.h
                src/support/logger.h
                src/support/mod_format.h
                src/support/platform.h
                src/support/types.h
                src/support/version_regex.h
                src/updating/updater.h)

# Main Source Files

set(BOSS_SRC    src/common/conditional_data.cpp
                src/common/error.cpp
                src/common/game.cpp
                src/common/globals.cpp
                src/common/item_list.cpp
                src/common/keywords.cpp
                src/common/rule_line.cpp
                src/common/settings.cpp
                src/output/boss_log.cpp
                src/output/output.cpp
                src/parsing/grammar.cpp
                src/support/helpers.cpp
                src/support/logger.cpp
                src/support/mod_format.cpp
                src/support/version_regex.cpp
                src/updating/updater.cpp)

# API Header Files

set(BOSS_API_H  ${BOSS_H}
                src/api/boss.h)

# API Source Files

set(BOSS_API_SRC ${BOSS_SRC}
                 src/api/boss.cpp)

# GUI Header Files

set(GUI_H       ${BOSS_H}
                src/gui/element_ids.h
                src/gui/main_window.h
                src/gui/settings_window.h
                src/gui/user_rule_editor.h)

# GUI Source Files

set(GUI_SRC     ${BOSS_SRC}
                src/gui/element_ids.cpp
                src/gui/main_window.cpp
                src/gui/settings_window.cpp
                src/gui/user_rule_editor.cpp)

#link_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib${ARCH})

if (NOT DEFINED ARCH)
	link_directories(${CMAKE_SOURCE_DIR}/lib)
else ()
	link_directories(${CMAKE_SOURCE_DIR}/lib${ARCH})
endif ()
configure_file(${CMAKE_SOURCE_DIR}/src/config.h.in ${CMAKE_SOURCE_DIR}/src/config.h)

# Find Boost
if (WIN32)
	# For Windows, only build static builds
	#set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Shared Library (OFF for Static)")
	set(BUILD_SHARED_LIBS OFF)
	if (MINGW)
		#add_compile_options("-static" "-static-libstdc++" "-static-libgcc")
		#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
		#set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--allow-multiple-definition")
		#set(CMAKE_EXE_LINKER_FLAGS "-static")
		#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static --enable-stdcall-fixup")

		set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static")
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

		# Temporary, need to figure out how to properly make a static build
		#set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
		#set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
		#set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC
		#set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
		#set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic
		#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
	endif ()
	# Set up the Boost options for Windows
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_STATIC_RUNTIME ON)
	if (CMAKE_GENERATOR STREQUAL "Visual Studio 14 2015")
		find_package(Boost REQUIRED filesystem regex program_options locale system exception)
	else ()
		find_package(Boost REQUIRED filesystem program_options locale system exception)
	endif ()
else ()
	find_package(Boost REQUIRED filesystem iostreams program_options locale system)
endif ()

add_executable(boss src/boss_cli.cpp ${BOSS_SRC})

target_include_directories(boss PUBLIC src)
target_include_directories(boss SYSTEM PUBLIC ${Boost_INCLUDE_DIRS} ${LIBGIT2_ROOT}/include)

# If building for Windows, add in the resource file
if (WIN32)
	target_sources(boss PUBLIC BOSS_version.rc)
endif ()

# For MinGW and GCC
if (CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options(boss PUBLIC "-O3" "-std=c++11")
	#if (WIN32)
		#target_compile_options(boss PUBLIC "-Wl,--enable-stdcall-fixup")
		#target_compile_options(boss PUBLIC "-municode")
	#endif()
	if (ENABLE_ALL_WARNINGS)
		target_compile_options(boss PUBLIC "-Weffc++" "-Wall" "-Wpedantic" "-Wextra")
	endif ()
	#if (WIN32 AND MINGW AND ARCH EQUAL 32)
		#target_compile_options(boss PUBLIC "-march=i686")
	#endif ()
endif ()

if(MINGW)
#target_compile_options(boss PUBLIC "-static-libstdc++" "-static-libgcc")
	set_target_properties(boss PROPERTIES LINK_SEARCH_START_STATIC 1)
	set_target_properties(boss PROPERTIES LINK_SEARCH_END_STATIC 1)
endif ()

# Determine architecture of the machine
#if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	#target_compile_definitions(boss PUBLIC -DBOSS_ARCH_64)
	#if (BUILD_GUI)
		#target_compile_definitions(boss_gui PUBLIC -DBOSS_ARCH_64)
	#endif()
	#set(USE_64 ON)
#elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
	#target_compile_definitions(boss PUBLIC -DBOSS_ARCH_32)
	#if (BUILD_GUI)
		#target_compile_definitions(boss_gui PUBLIC -DBOSS_ARCH_32)
	#endif()
	#set(USE_32 ON)
#else()
	#message(FATAL_ERROR "Unsupported architecture")
#endif()

target_link_libraries(boss ${Boost_LIBRARIES} git2 ssh2)

# Add Windows specific libraries
if (WIN32)
	target_link_libraries(boss version gdi32)
	#if (USE_64)
		#target_link_libraries(boss version gdi32)
	#elseif (USE_32)
		#target_link_libraries(boss gdi32)
	#endif()
endif ()

# GUI Build
if (BUILD_GUI)
	# Set up and find wxWidgets
	if (WIN32)
		set(wxWidgets_ROOT_DIR "../wxWidgets")
		set(wxWidgets_CONFIGURATION mswu)
		set(wxWidgets_USE_STATIC ON)
		set(wxWidgets_LIB_DIR ${CMAKE_SOURCE_DIR}/lib${ARCH})
		set(wxWidgets_CONFIG_EXECUTABLE "${CMAKE_SOURCE_DIR}/lib${ARCH}/wx-config")
	endif ()

	if (wxWidgets_FOUND)
		include(${wxWidgets_USE_FILE})
	endif ()
	set(wxWidgets_USE_UNICODE ON)
	find_package(wxWidgets COMPONENTS core base adv)
	message(STATUS "Using wxWidgets Static Libraries: " ${wxWidgets_USE_STATIC})
	message(STATUS "wxWidgets Include Directories: " ${wxWidgets_INCLUDE_DIRS})
	message(STATUS "Location of wx-config: " ${wxWidgets_CONFIG_EXECUTABLE})
	message(STATUS "wxWidgets Libraries: " ${wxWidgets_LIBRARIES})
	message(STATUS "wxWidgets Lib Directory: " ${wxWidgets_LIB_DIR})
	message(STATUS "wxWidgets Config Options: " ${wxWidgets_CONFIG_OPTIONS})
	#message(STATUS "Architecture: " ${CMAKE_CXX_LIBRARY_ARCHITECTURE})

	add_executable(boss_gui ${GUI_SRC})

	if(MINGW)
		set_target_properties(boss_gui PROPERTIES LINK_SEARCH_START_STATIC 1)
		set_target_properties(boss_gui PROPERTIES LINK_SEARCH_END_STATIC 1)
	endif ()
	target_include_directories(boss_gui PUBLIC src)
	target_include_directories(boss_gui SYSTEM PUBLIC ${Boost_INCLUDE_DIRS} ${LIBGIT2_ROOT}/include ${wxWidgets_INCLUDE_DIRS})

	# If building for Windows, add in the resource file
	if (WIN32)
		target_sources(boss_gui PUBLIC BOSS_version.rc)
	endif ()

	# For MinGW and GCC
	if (CMAKE_COMPILER_IS_GNUCXX)
		target_compile_options(boss_gui PUBLIC "-O3" "-std=c++11")
		#if (WIN32)
			#target_compile_options(boss_gui PUBLIC "-municode")
			#target_compile_options(boss_gui PUBLIC "-Wl,--enable-stdcall-fixup")
		#endif()
		if (ENABLE_ALL_WARNINGS)
			target_compile_options(boss_gui PUBLIC "-Weffc++" "-Wall" "-Wpedantic" "-Wextra")
		endif ()
	endif ()

	target_compile_definitions(boss_gui PUBLIC ${wxWidgets_DEFINITIONS})
	target_link_libraries(boss_gui ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} git2 ssh2)
	if (WIN32)
		target_link_libraries(boss_gui version gdi32)
		#if (USE_64)
			#target_link_libraries(boss_gui ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} git2)
			#target_compile_definitions(boss_gui PUBLIC -DBOSS_ARCH_64)
			#target_link_libraries(boss_gui version gdi32)
		#elseif (USE_32)
			#list(REMOVE_ITEM wxWidgets_LIBRARIES version)
			#target_link_libraries(boss_gui ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} git2)
			#target_compile_definitions(boss_gui PUBLIC -DBOSS_ARCH_32)
			#target_link_libraries(boss_gui gdi32)
		#endif()
	endif ()
	#if (WIN32 AND MINGW AND ARCH EQUAL 32)
		#target_compile_options(boss_gui PUBLIC "-march=i686")
	#endif ()
endif ()

# MinGW needs to link in zlib for libgit2 (v0.20.0)
if (MINGW)
	find_package(ZLIB REQUIRED)
#	find_package(HTTP_Parser REQUIRED)
	target_link_libraries(boss ${ZLIB_LIBRARIES} winhttp crypt32 rpcrt4)
	if (BUILD_GUI)
		target_link_libraries(boss_gui ${ZLIB_LIBRARIES} winhttp crypt32 rpcrt4)
	endif ()
endif ()

# MinGW and *nix platforms need to link in the OpenSSL libraries for libgit2 (v0.20.0)
if (MINGW OR NOT WIN32)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(boss ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
	if (BUILD_GUI)
		target_link_libraries(boss_gui ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
	endif ()
endif ()
if (WIN32)
	#if (ARCH EQUAL 32)
		#target_link_libraries(boss wsock32)
		#if (BUILD_GUI)
			#target_link_libraries(boss_gui wsock32)
		#endif ()
	#endif ()
	#if (ARCH EQUAL 32)
		#target_compile_options(boss PUBLIC "-Wl,--enable-stdcall-fixup")
		#if (BUILD_GUI)
			#target_compile_options(boss_gui PUBLIC "-Wl,--enable-stdcall-fixup")
		#endif ()
	#endif ()
	target_link_libraries(boss ws2_32)
	if (BUILD_GUI)
		target_link_libraries(boss_gui ws2_32)
	endif ()
endif ()

add_library(bapi${ARCH} ${BOSS_API_SRC} ${BOSS_API_H})

target_include_directories(bapi${ARCH} PUBLIC src)
target_include_directories(bapi${ARCH} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS} ${LIBGIT2_ROOT}/include)

# For MinGW and GCC
if (CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options(bapi${ARCH} PUBLIC "-O3" "-std=c++11")
	if (ENABLE_ALL_WARNINGS)
		target_compile_options(bapi${ARCH} PUBLIC "-Weffc++" "-Wall" "-Wpedantic" "-Wextra")
	endif ()
endif ()

target_link_libraries(bapi${ARCH} ${Boost_LIBRARIES} git2 ssh2)

if (WIN32)
	target_link_libraries(bapi${ARCH} version gdi32)
	if (MINGW)
		target_link_libraries(bapi${ARCH} ${ZLIB_LIBRARIES} winhttp crypt32 rpcrt4)
	endif ()
endif ()

# MinGW and *nix platforms need to link in the OpenSSL libraries for libgit2 (v0.20.0)
if (MINGW OR NOT WIN32)
	target_link_libraries(bapi${ARCH} ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
endif ()

if (WIN32)
	target_link_libraries(bapi${ARCH} ws2_32)
endif ()
