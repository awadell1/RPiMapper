OPENCV_CFLAGS = `pkg-config --cflags opencv`
OPENCV_LFLAGS = `pkg-config --libs opencv` 
ARTKP_CFLAGS = `pkg-config --cflags artoolkitplus`
ARTKP_LFLAGS = `pkg-config --libs artoolkitplus`
GSTREAMER_CFLAGS=`pkg-config --cflags gstreamer-0.10 gstreamer-app-0.10`
GSTREAMER_LFLAGS=`pkg-config --libs gstreamer-0.10 gstreamer-app-0.10`

CFLAGS 	= -O2 -Wall $(OPENCV_CFLAGS) $(ARTKP_CFLAGS) $(GSTREAMER_CFLAGS)
LDFLAGS = -L. -lrt -lpthread $(OPENCV_LFLAGS) $(ARTKP_LFLAGS) $(GSTREAMER_LFLAGS)
CPP	= g++

SRCS 	= main.cpp \
	  camera/ARtag.cpp \
	  camera/ARtagLocalizer.cpp \
	  camera/Camera.cpp \
	  create/Create.cpp \
	  sonar/Gpio.cpp \
	  sonar/Sonar.cpp 

OBJS 	= main.o \
	  ARtag.o \
	  ARtagLocalizer.o \
	  Camera.o \
	  Create.o \
	  Gpio.o \
	  Sonar.o 

EXECUTABLE = main

all	: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@ $*

main.o	: main.cpp
	$(CPP) $(CFLAGS) -c $^

ARtag.o	: camera/ARtag.h camera/ARtag.cpp
	$(CPP) $(CFLAGS) -c $^

ARtagLocalizer.o : camera/ARtagLocalizer.h camera/ARtag.h camera/ARtagLocalizer.cpp
	$(CPP) $(CFLAGS) -c $^

Camera.o : camera/ARtagLocalizer.h camera/ARtag.h camera/Camera.h camera/Camera.cpp
	$(CPP) $(CFLAGS) -c $^

Create.o : Packet.h create/Create.h create/Create.cpp
	$(CPP) $(CFLAGS) -c $^

Sonar.o	: sonar/Gpio.h sonar/Sonar.h sonar/Sonar.cpp
	$(CPP) $(CFLAGS) -c $^

Gpio.o : sonar/Gpio.h sonar/Gpio.cpp
	$(CPP) $(CFLAGS) -c $^

$(OBJS)	: Packet.h control/Control.h camera/ARtag.h camera/ARtagLocalizer.h create/Create.h sonar/Gpio.h sonar/Sonar.h 

clean	:
	rm -rf main *.o
	rm -rf *.*~
	rm -rf *.h.gch
