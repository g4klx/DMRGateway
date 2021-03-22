CC      = cc
CXX     = c++

# Use the following CFLAGS and LIBS if you don't want to use gpsd.
CFLAGS  = -g -O3 -Wall -DHAVE_LOG_H -std=c++0x -pthread
LIBS    = -lpthread

# Use the following CFLAGS and LIBS if you do want to use gpsd.
#CFLAGS  = -g -O3 -Wall -DHAVE_LOG_H -DUSE_GPSD -std=c++0x -pthread
#LIBS    = -lpthread -lgps

LDFLAGS = -g

OBJECTS = APRSWriter.o BPTC19696.o Conf.o CRC.o DMRCSBK.o DMRData.o DMRDataHeader.o DMREmbeddedData.o DMREMB.o DMRFullLC.o DMRGateway.o \
	  DMRLC.o DMRNetwork.o DMRSlotType.o DynVoice.o Golay2087.o GPSD.o Hamming.o Log.o MMDVMNetwork.o PassAllPC.o PassAllTG.o \
	  QR1676.o Reflectors.o RemoteControl.o Rewrite.o RewriteDstId.o RewriteDynTGNet.o RewriteDynTGRF.o RewritePC.o RewriteSrc.o RewriteSrcId.o \
	  RewriteTG.o RewriteType.o RS129.o SHA256.o StopWatch.o Sync.o Thread.o Timer.o UDPSocket.o Utils.o XLXVoice.o

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

install:
		install -m 755 DMRGateway /usr/local/bin/

# Export the current git version if the index file exists, else 000...
GitVersion.h:
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif

