find_package (Boost COMPONENTS system log filesystem unit_test_framework REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})
link_directories ( ${Boost_LIBRARY_DIRS} )
add_definitions (-DBOOST_ALL_DYN_LINK)
add_executable (Test "test.cpp")
target_link_libraries (Test
                       signatureLib
                       ${Boost_FILESYSTEM_LIBRARY}
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_LOG_LIBRARY}
                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                       )
add_test (NAME MyTest COMMAND Test)