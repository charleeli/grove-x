#!/bin/bash

../../../depends/bin/thrift --out ./gen_cpp   -r --gen cpp ../../idl/httpCmd.thrift
../../../depends/bin/thrift --out ./gen_py    -r --gen py  ../../idl/httpCmd.thrift

