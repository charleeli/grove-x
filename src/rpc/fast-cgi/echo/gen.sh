#!/bin/bash

../../../depends/bin/thrift --out ./sdk-cpp/echo  -r --gen cpp ../../idl/echoCmd.thrift
../../../depends/bin/thrift --out ./sdk-py/gen_py -r --gen py ../../idl/echoCmd.thrift

