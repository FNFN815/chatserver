#SRC_LIST指定当前目录下所有的源文件
aux_source_directory(. SRC_LIST)
#aux_source_directory(./db DB_LIST)
#aux_source_directory(./model MODEL_LIST)
#指定生成可执行文件
add_executable(ChatClient ${SRC_LIST} )

#连接库
target_link_libraries(ChatClient pthread)