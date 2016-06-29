include_directories("${PROJECT_SOURCE_DIR}/base")
include_directories("${PROJECT_SOURCE_DIR}/rpc/fast-cgi/")

set(COUPON_PUSH_SOURCES
    ./sdk-cpp/echo/echoCmd_constants.cpp
    ./sdk-cpp/echo/echoCmd_types.cpp
    ../util/gen_cpp/httpCmd_constants.cpp
    ../util/gen_cpp/httpCmd_types.cpp
)
add_library(echoCmd STATIC ${COUPON_PUSH_SOURCES})
target_link_libraries(echoCmd libthrift.a)

add_executable(echo_module main.cc )
target_link_libraries(echo_module echoCmd)
target_link_libraries(echo_module libglog.a libfcgi.a libredismq.a libhiredis.a)
target_link_libraries(echo_module ${ZLIB_LIBRARIES} mysqlclient_r rt pthread)
set_target_properties(echo_module PROPERTIES COMPILE_FLAGS "-std=c++11")

INSTALL(TARGETS echo_module	RUNTIME DESTINATION bin)
