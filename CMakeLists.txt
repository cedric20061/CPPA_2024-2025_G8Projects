cmake_minimum_required(VERSION 3.10)
project(MultiProject LANGUAGES C)

# Définir les options
set(CMAKE_C_STANDARD 11)

# Répertoires de sortie
set(CONSOLE_DIR ${CMAKE_BINARY_DIR}/console)
set(GUI_DIR ${CMAKE_BINARY_DIR}/GUI)
set(TESTS_DIR ${CMAKE_BINARY_DIR}/tests)

# Créer les répertoires de sortie
file(MAKE_DIRECTORY ${CONSOLE_DIR} ${GUI_DIR} ${TESTS_DIR})

# Trouver GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

# Ajouter les options de compilation GTK
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

# ========================
# Section Console
# ========================
add_executable(console_inventory src/console/inventory.c util/inventory.c util/global.c)
add_executable(console_game src/console/game.c util/game.c util/global.c)
add_executable(console_blocNote src/console/blocNote.c util/blocNote.c util/global.c)

# Configurer les cibles console
foreach(target console_inventory console_game console_blocNote)
    target_include_directories(${target} PRIVATE include)
    target_link_libraries(${target} m)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CONSOLE_DIR})
endforeach()

# ========================
# Section GUI
# ========================
add_executable(gui_inventory src/GUI/GUI_inventory.c util/inventory.c)
add_executable(gui_game src/GUI/GUI_game.c util/game.c)
add_executable(gui_blocNote src/GUI/GUI_blocNote.c util/blocNote.c)

# Configurer les cibles GUI
foreach(target gui_inventory gui_game gui_blocNote)
    target_include_directories(${target} PRIVATE include)
    target_link_libraries(${target} ${GTK_LIBRARIES})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${GUI_DIR})
endforeach()

# ========================
# Section Tests
# ========================
add_executable(test_inventory tests/inventory.c util/inventory.c)
add_executable(test_game tests/game.c util/game.c)
add_executable(test_blocNote tests/blocNote.c util/blocNote.c)

# Configurer les cibles tests
foreach(target test_inventory test_game test_blocNote)
    target_include_directories(${target} PRIVATE include)
    target_link_libraries(${target} m)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR})
endforeach()
