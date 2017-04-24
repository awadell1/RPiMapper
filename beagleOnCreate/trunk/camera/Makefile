OPENCV_CFLAGS = `pkg-config --cflags opencv`
OPENCV_LFLAGS = `pkg-config --libs opencv` 
ARTKP_CFLAGS = `pkg-config --cflags artoolkitplus`
ARTKP_LFLAGS = `pkg-config --libs artoolkitplus`
GSTREAMER_CFLAGS=`pkg-config --cflags gstreamer-0.10 gstreamer-app-0.10`
GSTREAMER_LFLAGS=`pkg-config --libs gstreamer-0.10 gstreamer-app-0.10`

CFLAGS 	= -O2 -Wall $(OPENCV_CFLAGS) $(ARTKP_CFLAGS) $(GSTREAMER_CFLAGS)
LDFLAGS = -L. -lpthread $(OPENCV_LFLAGS) $(ARTKP_LFLAGS) $(GSTREAMER_LFLAGS)
CPP	= g++

SRCS 	= example_main.cpp \
	  Camera.cpp \
	  ARtag.cpp \
	  ARtagLocalizer.cpp 

OBJS 	= example_main.o \
	  Camera.o \
	  ARtag.o \
	  ARtagLocalizer.o 

EXECUTABLE = example_main

all	: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@ $*

example_main.o	: example_main.cpp
	$(CPP) $(CFLAGS) -c $^

ARtag.o	: ARtag.h ARtag.cpp
	$(CPP) $(CFLAGS) -c $^

ARtagLocalizer.o : ARtagLocalizer.h ARtag.h ARtagLocalizer.cpp
	$(CPP) $(CFLAGS) -c $^
	
Camera.o : ARtagLocalizer.h ../Packet.h Camera.h Camera.cpp
	$(CPP) $(CFLAGS) -c $^

$(OBJS)	: ../Packet.h ARtag.h ARtagLocalizer.h Camera.h

clean	:
	rm -rf example_main *.o
	rm -rf *.*~
	rm -rf *.h.gch
