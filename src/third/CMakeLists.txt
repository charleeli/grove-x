include(ExternalProject)

ExternalProject_Add(libglog
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                https://github.com/google/glog/archive/v0.3.4.tar.gz
    URL_HASH           "MD5=df92e05c9d02504fb96674bc776a41cb"
    PATCH_COMMAND      patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/patch_glog/0001-redefine-the-format-of-log-file-name.patch  &&  patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/patch_glog/0002-output-to-a-new-file-per-hour.patch
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )

ExternalProject_Add(libevent
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                https://github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz
    URL_HASH           "MD5=b2405cc9ebf264aa47ff615d9de527a2"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#   INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )

ExternalProject_Add(libthrift
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://apache.fayea.com/thrift/0.9.2/thrift-0.9.2.tar.gz
    URL_HASH           "MD5=89f63cc4d0100912f4a1f8a9dee63678"
    CONFIGURE_COMMAND  env CFLAGS=-I${CMAKE_INSTALL_PREFIX}/include LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib ./configure --prefix=${CMAKE_INSTALL_PREFIX} --with-libevent=${CMAKE_INSTALL_PREFIX}/event2 --with-cpp=yes --with-qt4=no --with-c_glib=yes --with-csharp=no --with-java=no --with-erlang=no --with-nodejs=no --with-lua=no --with-python=no --with-perl=no --with-php=no --with-php_extension=no --with-ruby=no --with-haskell=no --with-go=no --with-d=no
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )

ExternalProject_Add(libhiredis
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                https://github.com/redis/hiredis/archive/v0.13.3.tar.gz
    URL_HASH           "MD5=43dca1445ec6d3b702821dba36000279"
    CONFIGURE_COMMAND  ""
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install "PREFIX=${CMAKE_INSTALL_PREFIX}"
    INSTALL_COMMAND    mkdir -p ${CMAKE_INSTALL_PREFIX}/include/hiredis ${CMAKE_INSTALL_PREFIX}/lib  &&  cp -a hiredis.h async.h read.h sds.h adapters ${CMAKE_INSTALL_PREFIX}/include/hiredis  &&  cp -a libhiredis.so libhiredis.a ${CMAKE_INSTALL_PREFIX}/lib/
    BUILD_IN_SOURCE    1
    )

ExternalProject_Add(libc-ares
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://c-ares.haxx.se/download/c-ares-1.10.0.tar.gz
    URL_HASH           "MD5=1196067641411a75d3cbebe074fd36d8"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )

ExternalProject_Add(libcurl
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://curl.haxx.se/download/curl-7.45.0.tar.bz2
    URL_HASH           "MD5=62c1a352b28558f25ba6209214beadc8"
    CONFIGURE_COMMAND  env CFLAGS=-I${CMAKE_INSTALL_PREFIX}/include LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib ./configure --prefix=${CMAKE_INSTALL_PREFIX} --enable-ares --disable-ldap --without-librtmp
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )

ExternalProject_Add(libmxml
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://www.msweet.org/files/project3/mxml-2.9.tar.gz
    URL_HASH           "MD5=e21cad0f7aacd18f942aa0568a8dee19"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(redlock
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://www.msweet.org/files/project3/redlock.tar.gz
    URL_HASH           "MD5=71574a6c4c3698ecc5a86930f8cbd908"
    CONFIGURE_COMMAND  ""
    BUILD_COMMAND      env CFLAGS=-I${CMAKE_INSTALL_PREFIX}/include LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib ${MAKE}
    INSTALL_COMMAND    mkdir -p ${CMAKE_INSTALL_PREFIX}/include/redlock ${CMAKE_INSTALL_PREFIX}/lib  &&  cp -a redlock-cpp/redlock.h redlock-cpp/sds.h  ${CMAKE_INSTALL_PREFIX}/include/redlock  &&  cp -a bin/libredlock.a ${CMAKE_INSTALL_PREFIX}/lib/
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(libev
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://dist.schmorp.de/libev/libev-4.22.tar.gz
    URL_HASH           "MD5=bf0007e37240103d3c2be80ca9bee3f9"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#   INSTALL_COMMAND    ${MAKE} install 
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(libodb
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://www.codesynthesis.com/download/odb/2.4/libodb-2.4.0.tar.gz
    URL_HASH           "MD5=e992e208aa18e7a61deeb88691299b64"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#   INSTALL_COMMAND    ${MAKE} install 
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(libodb-mysql
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://www.codesynthesis.com/download/odb/2.4/libodb-mysql-2.4.0.tar.gz
    URL_HASH           "MD5=7cafd26bda5e9db616c0ff302c33101f"
    CONFIGURE_COMMAND  env CFLAGS=-I${CMAKE_INSTALL_PREFIX}/include LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib ./configure --prefix=${CMAKE_INSTALL_PREFIX} --with-libodb=${CMAKE_INSTALL_PREFIX}/include
    BUILD_COMMAND      ${MAKE}
#   INSTALL_COMMAND    ${MAKE} install 
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(libodb-boost
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                http://www.codesynthesis.com/download/odb/2.4/libodb-boost-2.4.0.tar.gz
    URL_HASH           "MD5=78f467a88df86aba812ae50faed878d1"
    CONFIGURE_COMMAND  env CFLAGS=-I${CMAKE_INSTALL_PREFIX}/include LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib ./configure --prefix=${CMAKE_INSTALL_PREFIX} --with-libodb=${CMAKE_INSTALL_PREFIX}/include
    BUILD_COMMAND      ${MAKE}
#   INSTALL_COMMAND    ${MAKE} install 
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(fcgi
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                "http://fastcgi.com/dist/fcgi-2.4.1.tar.gz"
    URL_HASH           "MD5=0f06bc1f6bb5bd5e8da12ce00d9ce91c"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )
    
ExternalProject_Add(spawn-fcgi
    DOWNLOAD_DIR       ${PROJECT_SOURCE_DIR}/download
    URL                "http://www.lighttpd.net/download/spawn-fcgi-1.6.3.tar.gz"
    URL_HASH           "MD5=6d75f9e9435056fa1e574d836d823cd0"
    CONFIGURE_COMMAND  ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND      ${MAKE}
#    INSTALL_COMMAND    ${MAKE} install
    BUILD_IN_SOURCE    1
    )
      
