OPENCV_CFLAGS = `pkg-config --cflags opencv`
OPENCV_LFLAGS = `pkg-config --libs opencv` 
ARTKP_CFLAGS = `pkg-config --cflags artoolkitplus`
ARTKP_LFLAGS = `pkg-config --libs artoolkitplus`
GSTREAMER_CFLAGS=`pkg-config --cflags gstreamer-0.10 gstreamer-app-0.10`
GSTREAMER_LFLAGS=`pkg-config --libs gstreamer-0.10 gstreamer-app-0.10`

CFLAGS 	= -O2 -Wall $(OPENCV_CFLAGS) $(ARTKP_CFLAGS) $(GSTREAMER_CFLAGS)
LDFLAGS = -L. -lrt -lpthread $(OPENCV_LFLAGS) $(ARTKP_LFLAGS) $(GSTREAMER_LFLAGS)
CPP	= g++

SRCS 	= example_main.cpp \
	  Create.cpp 

OBJS 	= example_main.o \
	  Create.o 

EXECUTABLE = example_main

all	: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@ $*

example_main.o	: ../Packet.h Create.h example_main.cpp
	$(CPP) $(CFLAGS) -c $^

Create.o : ../Packet.h Create.h Create.cpp
	$(CPP) $(CFLAGS) -c $^

$(OBJS)	: ../Packet.h Create.h

clean	:
	rm -rf example_main *.o
	rm -rf *.*~
	rm -rf *.h.gch
