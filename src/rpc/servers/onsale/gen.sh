#!/bin/bash

echo "wait a few seconds..."
echo "thrift compiling..."

../../../depends/bin/thrift --out ./sdk-cpp/ec      -r --gen cpp ../../idl/onsaleEC.thrift
../../../depends/bin/thrift --out ./sdk-py/gen_py   -r --gen py  ../../idl/onsaleEC.thrift

../../../depends/bin/thrift --out ./sdk-cpp/admin   -r --gen cpp ../../idl/onsaleAdmin.thrift
../../../depends/bin/thrift --out ./sdk-py/gen_py   -r --gen py  ../../idl/onsaleAdmin.thrift

echo "thrift compile done."
echo "odb compiling..."

odb --database mysql \
	--profile boost \
	--profile boost/optional \
	--profile boost/smart-ptr \
	--profile boost/optional \
	--profile boost/date-time \
	--profile boost/date-time/gregorian \
	--profile boost/date-time/posix-time \
	--default-pointer boost::shared_ptr \
	--output-dir ./orm_onsale/ \
	--changelog-dir ./orm_onsale/ \
	--generate-prepared --generate-query --generate-session --generate-schema ../../orm/onsale.hxx

echo "odb compile done."
echo "all compile done!"