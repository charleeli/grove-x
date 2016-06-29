include_directories("${PROJECT_SOURCE_DIR}/base")
include_directories("${PROJECT_SOURCE_DIR}/rpc/orm")

set(ONSALE_ADMIN_SOURCES
    ./orm_onsale/onsale-odb.cxx
    ./sdk-cpp/admin/onsale_constants.cpp
    ./sdk-cpp/admin/onsale_types.cpp
    ./sdk-cpp/admin/onsaleAdmin_constants.cpp
    ./sdk-cpp/admin/onsaleAdmin_types.cpp
    ./sdk-cpp/admin/OnsaleAdminService.cpp
)
add_library(onsaleAdmin STATIC ${ONSALE_ADMIN_SOURCES})
target_link_libraries(onsaleAdmin libthrift.a libhiredis.a)
set_target_properties(onsaleAdmin PROPERTIES COMPILE_FLAGS "-Wno-unknown-pragmas -Wno-deprecated-declarations")

set(ONSALE_EC_SOURCES
    ./orm_onsale/onsale-odb.cxx
    ./sdk-cpp/ec/onsale_constants.cpp
    ./sdk-cpp/ec/onsale_types.cpp
    ./sdk-cpp/ec/onsaleEC_constants.cpp
    ./sdk-cpp/ec/onsaleEC_types.cpp
    ./sdk-cpp/ec/OnsaleECService.cpp
)
add_library(onsaleEC STATIC ${ONSALE_EC_SOURCES})
target_link_libraries(onsaleEC libthrift.a libhiredis.a)
set_target_properties(onsaleEC PROPERTIES COMPILE_FLAGS "-Wno-unknown-pragmas -Wno-deprecated-declarations")

add_executable(onsale_admin_server OnsaleAdminServer.cc )
target_link_libraries(onsale_admin_server onsaleAdmin)
target_link_libraries(onsale_admin_server libthrift.a )
target_link_libraries(onsale_admin_server libodb-mysql.a libodb-boost.a libodb.a -lstdc++)
target_link_libraries(onsale_admin_server libglog.a libhiredis.a libredlock.a mysqlclient_r)
target_link_libraries(onsale_admin_server ${ZLIB_LIBRARIES}  libthriftnb.a event rt pthread)
set_target_properties(onsale_admin_server PROPERTIES COMPILE_FLAGS "-std=c++11")
set_target_properties(onsale_admin_server PROPERTIES COMPILE_FLAGS "-Wno-unknown-pragmas -Wno-deprecated-declarations")

add_executable(onsale_ec_server OnsaleECServer.cc )
target_link_libraries(onsale_ec_server onsaleEC)
target_link_libraries(onsale_ec_server libthrift.a )
target_link_libraries(onsale_ec_server libodb-mysql.a libodb-boost.a libodb.a -lstdc++)
target_link_libraries(onsale_ec_server libglog.a libhiredis.a libredlock.a mysqlclient_r)
target_link_libraries(onsale_ec_server ${ZLIB_LIBRARIES}  libthriftnb.a event rt pthread)
set_target_properties(onsale_ec_server PROPERTIES COMPILE_FLAGS "-std=c++11")
set_target_properties(onsale_ec_server PROPERTIES COMPILE_FLAGS "-Wno-unknown-pragmas -Wno-deprecated-declarations")

add_executable(onsale_label_server OnsaleLabelServer.cc )
target_link_libraries(onsale_label_server onsaleEC)
target_link_libraries(onsale_label_server onsaleAdmin)
target_link_libraries(onsale_label_server libthrift.a )
target_link_libraries(onsale_label_server libredismq.a libev.a )
target_link_libraries(onsale_label_server libodb-mysql.a libodb-boost.a libodb.a -lstdc++)
target_link_libraries(onsale_label_server libglog.a libhiredis.a libredlock.a mysqlclient_r)
target_link_libraries(onsale_label_server ${ZLIB_LIBRARIES}  libthriftnb.a event rt pthread)
set_target_properties(onsale_label_server PROPERTIES COMPILE_FLAGS "-std=c++11")
set_target_properties(onsale_label_server PROPERTIES COMPILE_FLAGS "-Wno-unknown-pragmas -Wno-deprecated-declarations")

INSTALL(TARGETS onsale_admin_server     RUNTIME DESTINATION bin)
INSTALL(TARGETS onsale_ec_server        RUNTIME DESTINATION bin)
INSTALL(TARGETS onsale_label_server     RUNTIME DESTINATION bin)
