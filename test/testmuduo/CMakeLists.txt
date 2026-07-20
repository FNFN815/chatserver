cmake_minimum_required(VERSION 3.0)
PROJECT(MAIN)
#配置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#头文件搜索路径
# include_directories()

#库文件搜索路径
# link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./muduo_server.cpp)

#设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#把.指定路径下的所有.cpp文件加入到SRC_LIST变量中
#aux_source_directory(. SRC_LIST)

#可执行文件生成,由SRC_LIST变量指定需要编译的源文件列表编译而来
add_executable(server ${SRC_LIST})

#server依赖的库文件 muduo_net muduo_base pthread
target_link_libraries(server muduo_net muduo_base pthread)