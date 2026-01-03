cmake_minimum_required(VERSION 3.16)
project(DesktopLauncher LANGUAGES CXX)

# C++17 标准（你用了 std::filesystem）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 MOC/UIC/RCC（Qt 元对象系统）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt6 所需组件
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

# 创建可执行文件
add_executable(DesktopLauncher main.cpp)

# 链接 Qt 库
target_link_libraries(DesktopLauncher
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# 可选：设置可执行文件输出目录（方便调试）
set_target_properties(DesktopLauncher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)