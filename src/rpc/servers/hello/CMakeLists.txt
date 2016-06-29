find_program(THRIFT_COMPILER thrift)
include_directories("${PROJECT_SOURCE_DIR}/base")
include_directories("${PROJECT_SOURCE_DIR}/rpc/orm")

set(HELLO_SOURCES 
    ./person/person-odb.cxx
    ./driver.cxx
)

add_executable(test_hello ${HELLO_SOURCES} )
target_link_libraries(test_hello libthrift.a libthriftnb.a)
target_link_libraries(test_hello libglog.a libredismq.a libev.a libhiredis.a)
target_link_libraries(test_hello libodb-mysql.a libodb-boost.a libodb.a -lstdc++)
target_link_libraries(test_hello ${ZLIB_LIBRARIES} mysqlclient_r event rt pthread)
set_target_properties(test_hello PROPERTIES COMPILE_FLAGS "-std=c++11 -Wno-unknown-pragmas -Wno-deprecated-declarations")

INSTALL(TARGETS test_hello      RUNTIME DESTINATION bin)
