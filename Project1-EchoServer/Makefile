#
# Makefile
# Computer Networking Programing Assignments
#
#  Created by Phillip Romig on 4/3/12.
#  Copyright 2012 Colorado School of Mines. All rights reserved.
#

CXX = g++
LD = g++
CXXFLAGS = -g -std=c++11 -DBOOST_LOG_DYN_LINK
LDFLAGS = -g 

#
# You should be able to add object files here without changing anything else
#
TARGET = echo_s
OBJ_FILES = ${TARGET}.o
INC_FILES = ${TARGET}.h

#
# Any libraries we might need.
#
LIBRARYS = -lboost_log_setup -lboost_log -lboost_thread -lboost_system  -lpthread


${TARGET}: ${OBJ_FILES}
	${LD} ${LDFLAGS} ${OBJ_FILES} -o $@ ${LIBRARYS}

%.o : %.cc ${INC_FILES}
	${CXX} -c ${CXXFLAGS} -o $@ $<

#
# Please remember not to submit objects or binarys.
#
clean:
	rm -f core ${TARGET} ${OBJ_FILES}

#
# This might work to create the submission tarball in the formal I asked for.
#
submit:
	rm -rf core project1 `whoami` ${OBJ_FILES}
	mkdir `whoami`
	cp Makefile README.txt *.h *.cc `whoami`
	tar cfv `whoami`.tar `whoami`
