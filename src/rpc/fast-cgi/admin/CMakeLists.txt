include_directories("${PROJECT_SOURCE_DIR}/base")
include_directories("${PROJECT_SOURCE_DIR}/base/libweb")
include_directories("${PROJECT_SOURCE_DIR}/rpc/fast-cgi/")

add_executable(admin_module main.cc )
target_link_libraries(admin_module libweb.a libglog.a )
target_link_libraries(admin_module libfcgi.a libredismq.a libhiredis.a)
target_link_libraries(admin_module ${ZLIB_LIBRARIES} mysqlclient_r rt pthread)

INSTALL(TARGETS admin_module	RUNTIME DESTINATION bin)
