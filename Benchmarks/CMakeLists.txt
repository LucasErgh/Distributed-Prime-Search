add_executable(SearchBenchmark "src/searchBenchmark.cpp")
set_target_properties(SearchBenchmark PROPERTIES OUTPUT_NAME "SearchBenchmarks")
target_include_directories(SearchBenchmark PUBLIC include)
target_link_libraries(SearchBenchmark PRIVATE Client)

add_custom_target(benchmark_search
    COMMENT "Run search benchmark"
    COMMAND $<TARGET_FILE:SearchBenchmark>
    USES_TERMINAL
    DEPENDS Client
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)