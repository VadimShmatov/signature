find_package (Boost COMPONENTS system log REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})
link_directories ( ${Boost_LIBRARY_DIRS} )
add_definitions (-DBOOST_ALL_DYN_LINK)
add_library (signatureLib "Task.cpp"
                          "FileBlockReader.cpp"
                          "FileBlockHasherMD5.cpp"
                          "FileBlockHashWriter.cpp"
                          "data/FileBlockHashBuffer.cpp")
add_executable (signature  "Main.cpp")
target_link_libraries (signature
                       signatureLib
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_LOG_LIBRARY})