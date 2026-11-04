add_executable(render-par)

target_sources(render-par 
    PRIVATE 
      src/main.cpp
      src/par_image.cpp
)

target_include_directories(render-par PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link with common library (which already links TBB)
target_link_libraries(render-par PRIVATE common)
