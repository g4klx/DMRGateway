# This makefile is for all platforms, but doesn't include support for the HD44780 display on the Raspberry Pi.

CC      = gcc
CXX     = g++
CFLAGS  = -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lpthread
LDFLAGS = -g

OBJECTS = Conf.o DMRData.o DMRGateway.o DMRNetwork.o Log.o MMDVMNetwork.o SHA256.o StopWatch.o Thread.o Timer.o UDPSocket.o Utils.o

all:	DMRGateway

DMRGateway:	$(OBJECTS) 
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o DMRGateway

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

clean:
		$(RM) DMRGateway *.o *.d *.bak *~
