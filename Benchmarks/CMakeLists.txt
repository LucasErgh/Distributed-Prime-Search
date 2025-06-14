project(Benchmarks LANGUAGES CXX)

add_executable(Benchmarks "src/searchBenchmark.cpp")
set_target_properties(Benchmarks PROPERTIES OUTPUT_NAME "DPPBenchmarks")

target_include_directories(Benchmarks PUBLIC include)

target_link_libraries(Benchmarks PUBLIC srcClient)