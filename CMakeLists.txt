cmake_minimum_required(VERSION 3.16)
project(websocket_trading_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------
# Main executable
# -------------------------
add_executable(trading_engine
    src/main.cpp
    src/orderbook.cpp
)

target_include_directories(trading_engine PRIVATE ${PROJECT_SOURCE_DIR}/include)

# -------------------------
# GoogleTest setup (portable)
# -------------------------
# Add GoogleTest subdirectory
add_subdirectory(extern/googletest)

# Test executable
add_executable(OrderBookTests
    tests/test_orderbook.cpp
    tests/test_order.cpp
    src/orderbook_learn.cpp
    src/orderbook.cpp
)

target_include_directories(OrderBookTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(OrderBookTests PRIVATE
    gtest_main
    pthread
)

# Enable CTest integration
enable_testing()
add_test(NAME OrderBookTests COMMAND OrderBookTests)
