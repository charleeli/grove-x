find_program(THRIFT_COMPILER thrift)
include_directories("${PROJECT_SOURCE_DIR}/base")

set(COUPON_EC_THRIFT ../../idl/couponEC.thrift)
set(COUPON_ADMIN_THRIFT ../../idl/couponAdmin.thrift)
set(COUPON_PUSH_THRIFT ../../idl/couponPush.thrift)

#if(THRIFT_COMPILER)
#execute_process(COMMAND ${THRIFT_COMPILER}  --out ./sdk-cpp/ec -r --gen cpp ${COUPON_EC_THRIFT}
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#execute_process(COMMAND ${THRIFT_COMPILER}  --out ./sdk-py/gen_py -r --gen py ${COUPON_EC_THRIFT}
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#                
#execute_process(COMMAND ${THRIFT_COMPILER}  --out ./sdk-cpp/admin -r --gen cpp ${COUPON_ADMIN_THRIFT}
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})  
#execute_process(COMMAND ${THRIFT_COMPILER}  --out ./sdk-py/gen_py -r --gen py ${COUPON_ADMIN_THRIFT}
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})     
#                
#execute_process(COMMAND ${THRIFT_COMPILER}  --out ./sdk-cpp/push -r --gen cpp ${COUPON_PUSH_THRIFT}
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})  
#execute_process(COMMAND ${THRIFT_COMPILER}  --out ./sdk-py/gen_py -r --gen py ${COUPON_PUSH_THRIFT}
#                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}) 
#endif()

set(COUPON_PUSH_SOURCES 
    ./sdk-cpp/push/coupon_constants.cpp
    ./sdk-cpp/push/coupon_types.cpp
    ./sdk-cpp/push/CouponPushService.cpp
    ./sdk-cpp/push/couponPush_constants.cpp
    ./sdk-cpp/push/couponPush_types.cpp
)
add_library(couponPush STATIC ${COUPON_PUSH_SOURCES})
target_link_libraries(couponPush libthrift.a)

set(COUPON_EC_SOURCES 
    ./sdk-cpp/ec/coupon_constants.cpp
    ./sdk-cpp/ec/coupon_types.cpp
    ./sdk-cpp/ec/CouponECService.cpp
    ./sdk-cpp/ec/couponEC_constants.cpp
    ./sdk-cpp/ec/couponEC_types.cpp
)
add_library(couponEC STATIC ${COUPON_EC_SOURCES})
target_link_libraries(couponEC libthrift.a libhiredis.a)

set(COUPON_ADMIN_SOURCES 
    ./sdk-cpp/admin/coupon_constants.cpp
    ./sdk-cpp/admin/coupon_types.cpp
    ./sdk-cpp/admin/CouponAdminService.cpp
    ./sdk-cpp/admin/couponAdmin_constants.cpp
    ./sdk-cpp/admin/couponAdmin_types.cpp
)
add_library(couponAdmin STATIC ${COUPON_ADMIN_SOURCES})
target_link_libraries(couponAdmin libthrift.a)

add_executable(coupon_ec_server CouponECServer.cc )
target_link_libraries(coupon_ec_server couponEC)
target_link_libraries(coupon_ec_server couponPush)
target_link_libraries(coupon_ec_server couponAdmin)
target_link_libraries(coupon_ec_server libthrift.a )
target_link_libraries(coupon_ec_server libglog.a libhiredis.a libredlock.a mysqlclient_r)
target_link_libraries(coupon_ec_server ${ZLIB_LIBRARIES}  libthriftnb.a event rt pthread)
set_target_properties(coupon_ec_server PROPERTIES COMPILE_FLAGS "-std=c++11")

add_executable(coupon_admin_server CouponAdminServer.cc )
target_link_libraries(coupon_admin_server couponEC)
target_link_libraries(coupon_admin_server couponPush)
target_link_libraries(coupon_admin_server couponAdmin)
target_link_libraries(coupon_admin_server libthrift.a)
target_link_libraries(coupon_admin_server libglog.a libredismq.a libev.a libhiredis.a mysqlclient_r)
target_link_libraries(coupon_admin_server ${ZLIB_LIBRARIES}  libthriftnb.a event rt pthread)
set_target_properties(coupon_admin_server PROPERTIES COMPILE_FLAGS "-std=c++11")

add_executable(coupon_push_server CouponPushServer.cc )
target_link_libraries(coupon_push_server couponEC)
target_link_libraries(coupon_push_server couponPush)
target_link_libraries(coupon_push_server couponAdmin)
target_link_libraries(coupon_push_server libthrift.a)
target_link_libraries(coupon_push_server libglog.a libhiredis.a mysqlclient_r)
target_link_libraries(coupon_push_server ${ZLIB_LIBRARIES}  libthriftnb.a event rt pthread)
set_target_properties(coupon_push_server PROPERTIES COMPILE_FLAGS "-std=c++11")

add_executable(coupon_batch_dispatch_server CouponBatchDispatchServer.cc )
target_link_libraries(coupon_batch_dispatch_server couponEC)
target_link_libraries(coupon_batch_dispatch_server couponPush)
target_link_libraries(coupon_batch_dispatch_server couponAdmin)
target_link_libraries(coupon_batch_dispatch_server libthrift.a libthriftnb.a)
target_link_libraries(coupon_batch_dispatch_server libglog.a libredismq.a libev.a libhiredis.a)
target_link_libraries(coupon_batch_dispatch_server ${ZLIB_LIBRARIES} mysqlclient_r event rt pthread)
set_target_properties(coupon_batch_dispatch_server PROPERTIES COMPILE_FLAGS "-std=c++11")

add_executable(coupon_batch_export_server CouponBatchExportServer.cc )
target_link_libraries(coupon_batch_export_server couponEC)
target_link_libraries(coupon_batch_export_server couponPush)
target_link_libraries(coupon_batch_export_server couponAdmin)
target_link_libraries(coupon_batch_export_server libthrift.a libthriftnb.a)
target_link_libraries(coupon_batch_export_server libglog.a libredismq.a libev.a libhiredis.a)
target_link_libraries(coupon_batch_export_server ${ZLIB_LIBRARIES} mysqlclient_r event rt pthread)
set_target_properties(coupon_batch_export_server PROPERTIES COMPILE_FLAGS "-std=c++11")

INSTALL(TARGETS coupon_ec_server                RUNTIME DESTINATION bin)
INSTALL(TARGETS coupon_admin_server             RUNTIME DESTINATION bin)
INSTALL(TARGETS coupon_push_server              RUNTIME DESTINATION bin)
INSTALL(TARGETS coupon_batch_dispatch_server    RUNTIME DESTINATION bin)
INSTALL(TARGETS coupon_batch_export_server      RUNTIME DESTINATION bin)

SET(TEST_COUPON "")
if(TEST_COUPON)
ADD_SUBDIRECTORY(test)
endif()

