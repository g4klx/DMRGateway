CC      = gcc
CXX     = g++
CFLAGS  = -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lpthread
LDFLAGS = -g

OBJECTS = BPTC19696.o Conf.o CRC.o DMRCSBK.o DMRData.o DMRDataHeader.o DMREmbeddedData.o DMREMB.o DMRFullLC.o DMRGateway.o DMRLC.o DMRNetwork.o DMRSlotType.o \
					Golay2087.o Hamming.o Log.o MMDVMNetwork.o PassAllPC.o PassAllTG.o QR1676.o Reflectors.o RepeaterProtocol.o Rewrite.o RewritePC.o RewriteSrc.o RewriteTG.o \
					RewriteType.o RS129.o SHA256.o StopWatch.o Sync.o Thread.o Timer.o UDPSocket.o Utils.o Voice.o

all:	DMRGateway

DMRGateway:	GitVersion.h $(OBJECTS) 
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o DMRGateway

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

DMRGateway.o: GitVersion.h FORCE

.PHONY: GitVersion.h

FORCE:

clean:
		$(RM) DMRGateway *.o *.d *.bak *~ GitVersion.h

# Export the current git version if the index file exists, else 000...
GitVersion.h:
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif

