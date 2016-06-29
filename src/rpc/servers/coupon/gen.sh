#!/bin/bash

echo "wait a few seconds..."
echo "thrift compiling..."

../../../depends/bin/thrift --out ./sdk-cpp/ec -r --gen cpp ../../idl/couponEC.thrift
../../../depends/bin/thrift --out ./sdk-py/gen_py -r --gen py ../../idl/couponEC.thrift

../../../depends/bin/thrift --out ./sdk-cpp/admin -r --gen cpp ../../idl/couponAdmin.thrift
../../../depends/bin/thrift --out ./sdk-py/gen_py -r --gen py ../../idl/couponAdmin.thrift

../../../depends/bin/thrift --out ./sdk-cpp/push -r --gen cpp ../../idl/couponPush.thrift
../../../depends/bin/thrift --out ./sdk-py/gen_py -r --gen py ../../idl/couponPush.thrift

echo "thrift compile done."
