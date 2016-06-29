#!/bin/bash

odb --database mysql \
	--profile boost \
	--profile boost/optional \
	--profile boost/smart-ptr \
	--profile boost/optional \
	--profile boost/date-time \
	--profile boost/date-time/gregorian \
	--profile boost/date-time/posix-time \
	--default-pointer boost::shared_ptr \
	--output-dir ./person/ \
	--changelog-dir ./person/ \
	--generate-prepared --generate-query --generate-session --generate-schema ../../orm/person.hxx

