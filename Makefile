CC      = cc
CXX     = c++

# Use the following CFLAGS and LIBS if you don't want to use gpsd.
CFLAGS  = -g -O3 -Wall -std=c++0x -MMD -MD -pthread
LIBS    = -lpthread -lmosquitto

# Use the following CFLAGS and LIBS if you do want to use gpsd.
#CFLAGS  = -g -O3 -Wall -DUSE_GPSD -std=c++0x -MMD -MD -pthread
#LIBS    = -lpthread -lgps -lmosquitto

LDFLAGS = -g

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all:	DMRGateway

DMRGateway:	GitVersion.h $(OBJS) 
		$(CXX) $(OBJS) $(CFLAGS) $(LIBS) -o DMRGateway

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<
-include $(DEPS)

DMRGateway.o: GitVersion.h FORCE

.PHONY: GitVersion.h

FORCE:

clean:
		$(RM) DMRGateway *.o *.d *.bak *~ GitVersion.h

install:
		install -m 755 DMRGateway /usr/local/bin/

# Export the current git version if the index file exists, else 000...
GitVersion.h:
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif

