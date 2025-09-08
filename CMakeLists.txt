cmake_minimum_required(VERSION 3.21.0...3.22.1)

project(
    voxel-engine
    VERSION 0.1.0
    DESCRIPTION "Cubic voxel engine for C++23."
    HOMEPAGE_URL "https://github.com/DmitriBogdanov/voxel-engine"
)

#add_subdirectory("ratlib/")

# =======================
# --- Fetch libraries ---
# =======================

include(cmake/CPM.cmake)

CPMAddPackage("gh:odygrd/quill#v10.1.0")
CPMAddPackage("gh:jeremy-rifkin/libassert#v2.2.1")
CPMAddPackage("gh:SFML/SFML#3.0.1")

# ==============
# --- RatLib ---
# ==============

add_library               (ratlib INTERFACE                  )
target_include_directories(ratlib INTERFACE "ratlib/include/")
target_compile_features   (ratlib INTERFACE cxx_std_20       )

target_link_libraries(ratlib INTERFACE
    quill
    libassert::assert
    sfml-system
    sfml-window
    sfml-graphics
)

# ===================
# --- Main target ---
# ===================

add_executable(
    engine

    "source/main.cpp"
)

target_include_directories(engine PRIVATE "include"             )
target_compile_features   (engine PRIVATE ${VE_COMPILE_FEATURES})
target_compile_options    (engine PRIVATE ${VE_COMPILER_FLAGS}  )
target_link_options       (engine PRIVATE ${VE_LINKER_FLAGS}    )
target_link_libraries     (engine PRIVATE ratlib                )

set_target_properties(engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
