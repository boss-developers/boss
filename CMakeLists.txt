cmake_minimum_required(VERSION 2.6)
project(boss)

option(BUILD_GUI "Build the BOSS GUI" ON)

set (LIBGIT2_ROOT "../libgit2")

set(SRC_H       src/common/conditional_data.h
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
                
set(SRC_BOSS    src/common/conditional_data.cpp
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
                src/support/version_regex.cpp)

set(SRC_GUI_H   ${SRC_H}
                src/gui/element_ids.h
                src/gui/main_window.h
                src/gui/settings_window.h
                src/gui/user_rule_editor.h)

set(SRC_GUI     ${SRC_BOSS}
                src/gui/element_ids.cpp
                src/gui/main_window.cpp
                src/gui/settings_window.cpp
                src/gui/user_rule_editor.cpp)

link_directories(${CMAKE_SOURCE_DIR}/lib)

if (WIN32)
	set(BUILD_SHARED_LIBS OFF)
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_STATIC_RUNTIME ON)
	find_package(Boost REQUIRED filesystem program_options locale system exception)
else ()
	find_package(Boost REQUIRED filesystem program_options locale system)
endif ()

include_directories(src ${Boost_INCLUDE_DIRS} ${LIBGIT2_ROOT}/include)

#add_executable(boss src/boss_cli.cpp ${SRC_H} ${SRC_BOSS})
add_executable(boss src/boss_cli.cpp ${SRC_BOSS})

target_compile_options(boss PUBLIC "-O3" "-std=c++11")

if (WIN32)
	target_link_libraries(boss ${Boost_LIBRARIES} version git2 gdi32 ws2_32)
else ()
	target_link_libraries(boss ${Boost_LIBRARIES} git2)
endif ()



if (BUILD_GUI)
	if (WIN32)
		set(wxWidgets_CONFIGURATION mswu)
		set(wxWidgets_USE_STATIC ON)
	endif ()
	if (wxWidgets_FOUND)
		include(${wxWidgets_USE_FILE})
	endif ()
	set(wxWidgets_USE_UNICODE ON)
	find_package(wxWidgets COMPONENTS core base adv)
	include_directories(${wxWidgets_INCLUDE_DIRS})
	add_executable(boss_gui ${SRC_GUI_H} ${SRC_GUI})
	target_compile_options(boss_gui PUBLIC "-O3" "-std=c++11" ${wxWidgets_CXX_FLAGS})
	target_compile_definitions(boss_gui PUBLIC ${wxWidgets_DEFINITIONS})
	if (WIN32)
		target_link_libraries(boss_gui ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} version git2 gdi32 ws2_32)
	else ()
		target_link_libraries(boss_gui ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} git2)
	endif ()
endif ()

if (MINGW)
	find_package(ZLIB REQUIRED)
	target_link_libraries(boss ${ZLIB_LIBRARIES})
	if (BUILD_GUI)
		target_link_libraries(boss_gui ${ZLIB_LIBRARIES})
	endif ()
endif ()

if (MINGW OR NOT WIN32)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(boss ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
	if (BUILD_GUI)
		target_link_libraries(boss_gui ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
	endif ()
endif ()

#target_link_libraries(boss boost_filesystem boost_program_options boost_system boost_locale git2 ssl crypto)
#target_link_libraries(boss_gui ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} git2 ssl crypto)
