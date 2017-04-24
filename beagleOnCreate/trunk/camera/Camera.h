#ifndef CAMERA_H
#define CAMERA_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "ARtagLocalizer.h"

/*! \file Camera.h
 */

/*! The udp port number for sending the video stream to remote host. */
#define VIDEO_PORT 8855
/*! The udp port number for sending ARtag id and pose info. */
#define ARTAG_PORT 8844

/*! Set the app source name for gstreamer. */
#define APPSRCNAME "app-src_01"
/*! Set the app sink name for gstreamer. */
#define APPSINKNAME "app-sink_01"
/*! Set the image width used for gstreamer. */
#define IMG_WIDTH 320
/*! Set the image height used for gstreamer. */
#define IMG_HEIGHT 240

class Camera
{
public:
	Camera(int remoteSock, struct sockaddr_in & videoPort, struct sockaddr_in & artagPort);
	~Camera();
	
	int StreamARtagVideo();
	void QuitMainLoop();
	void SendImage(IplImage * image);
	void SendARtag();
	void SetVideoBroadcast(bool isBroadcast);
	bool isBroadcast();

	/*! Pipeline 1 for gstreamer. */
	GstElement *pipeline1;
	/*! Pipeline 2 for gstreamer. */
	GstElement *pipeline2;
	/*! The image variable to be used for the buffer. */
	IplImage * img;
	/*! The gray image variable to be used for the buffer. */
	IplImage * gray;
	/*! The raw data from the image buffer. */
	uchar * IMG_data;
	/*! The artag localizer object. */
	ARtagLocalizer * ar;
	
private:
	int Setup();
	void CleanUp();

	int _sock;
	struct sockaddr_in _videoPort;
	struct sockaddr_in _artagPort;
	bool _isBroadcast;
	bool _isEnding;
};

#endif
