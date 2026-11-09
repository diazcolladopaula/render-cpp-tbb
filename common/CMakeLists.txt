add_library(common STATIC)

target_sources(common 
    PRIVATE 
        src/vector.cpp
        src/ray.cpp
        src/parsers.cpp
        src/objects.cpp
        src/material.cpp
        src/camera.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link with TBB
target_link_libraries(common PUBLIC Microsoft.GSL::GSL TBB::tbb)