#include <stdio.h>
#include "ARtagLocalizer.h"
/*! \file ARtagLocalizer.cpp
 */

/*!
  Set the fudge factor for multiplying the ARtag pose.
*/
#define FUDGE_FACTOR 0.97

using namespace std;

/*! The border thickness in meter when using thin artag pattern. */
const float THIN_PATTERN_BORDER = 0.125;
/*! The border thickness in meter when using thick artag pattern. */
const float THICK_PATTERN_BORDER = 0.25;

bool ARtagLocalizer::allStop = false;

/*! The calibration file path string, don't forget to change this when you change your camera.
 */
char calibFilename[] = "MS_LifeCam_VX700.cal";

/*!
 * 	\class ARtagLocalizer ARtagLocalizer.h "ARtagLocalizer.h"
 *	\brief This class is the core class that localize the ARtag using ARToolKitPlus.
 */

/*!	\fn ARtagLocalizer::ARtagLocalizer()
 * 	\brief A constructor for ARtagLocalizer.
 * 	Initalize image width and height, the flag to use BCH, patter center, pattern width, and fudge factor.
 */
ARtagLocalizer::ARtagLocalizer()
{
	imgwidth = 640;
	imgheight = 480;
	init = false;
	useBCH = true;
	patternCenter_[0] = patternCenter_[1] = 0.0;
	patternWidth_ = 80.0;
	xoffset = 0;
	yoffset = 0;
	fudge = FUDGE_FACTOR;
	arThreshold = 80;
}

/*!	\fn ARtagLocalizer::~ARtagLocalizer()
 * 	\brief A destructor for ARtagLocalizer.
 * 	There is nothing done in the destructor.
 */
ARtagLocalizer::~ARtagLocalizer()
{
}

/*!	\fn int ARtagLocalizer::initARtagPose(int width, int height, float markerWidth, float x_offset, float y_offset, float yaw_offset, float ffactor)
 * 	\brief Initalize The localizer with all these parameters.
 * 	\param width the width of the image input.
 * 	\param height the height of the image input.
 * 	\param markerWidth the width of the ARtag in CM which determines the size of the ARtag used.
 *  \param arThres the threshold value for binary thresholding the gray scale image. i.e. value < arThres = black else white
 * 	\param x_offset the offset of x in meter.
 * 	\param y_offset the offset of y in meter.
 *  \param ffactor the fudge factor to be multiplied on x and y estimation.
 * 	\return 0 on success -1 if failed.
 */
int ARtagLocalizer::initARtagPose(int width, int height, float markerWidth, int arThres, float x_offset, float y_offset, float yaw_offset, float ffactor)
{
	// create a tracker that does:
	//  - 6x6 sized marker images
	//  - samples at a maximum of 6x6
	//  - works with luminance (gray) images
	//  - can load a maximum of 1 pattern
	//  - can detect a maximum of 8 patterns in one image
    	tracker = new ARToolKitPlus::TrackerSingleMarker(width,height);//<6,6,6, 1, 8>(width,height);
	imgwidth = width;
	imgheight = height;
	patternCenter_[0] = patternCenter_[1] = 0.0;
	setARtagOffset(x_offset, y_offset, yaw_offset);
	fudge = ffactor;
	arThreshold = arThres;

	tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_LUM);
	// load a camera file. 
	if(!tracker->init(calibFilename, 1.0f, 1000.0f))
	{
		printf("ERROR: init() failed\n");
		delete tracker;
		return -1;
	}
	//printf("tracker init\n");
	patternWidth_ = markerWidth;
	// define size of the marker
    	tracker->setPatternWidth(patternWidth_);

	// the marker in the BCH test image has a thin border...
	tracker->setBorderWidth(THIN_PATTERN_BORDER);

	// set a threshold. alternatively we could also activate automatic thresholding
	tracker->setThreshold(arThreshold);
	tracker->activateAutoThreshold(true);

	// let's use lookup-table undistortion for high-speed
	// note: LUT only works with images up to 1024x1024
	tracker->setUndistortionMode(ARToolKitPlus::UNDIST_LUT);

	// RPP is more robust than ARToolKit's standard pose estimator but uses more CPU resource
	// so using standard pose estimator instead
	tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_ORIGINAL);
	//tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);

	// switch to simple ID based markers
	// use the tool in tools/IdPatGen to generate markers
	tracker->setMarkerMode(useBCH ? ARToolKitPlus::MARKER_ID_BCH : ARToolKitPlus::MARKER_ID_SIMPLE);
	//printf("finished init\n");
	init = true;
	return 0;
}

/*! \fn bool ARtagLocalizer::getARtagPose(IplImage* src, IplImage* dst, int camID)
 * 	\brief The function to check if there is any ARtag in the view. Also put in the tag id and pose on the image if found.
 * 	\param src the src image object.
 * 	\param dst the dst image object.
 * 	\param camID the camera to check.
 * 	\return true if found any, false otherwise.
 */
bool ARtagLocalizer::getARtagPose(IplImage* src, IplImage* dst, int camID)
{
	if (!init)
	{
		printf("Did not initalize the ARtagLocalizer!!\n");
		return NULL;
	}
	if (src->width != imgwidth || src->height != imgheight)
	{
		printf("Image passed in (%d,%d) does not match initialized image size (%d,%d)!!\n", src->width, src->height, imgwidth, imgheight);
		return NULL;
	}
	if (src->nChannels != 1)
	{
		printf("Please pass in grayscale image into ARtagLocalizer! \n");
		return NULL;
	}
	
	int numMarkers = 0;
	ARToolKitPlus::ARMarkerInfo* markers = NULL;
	if (tracker->arDetectMarker(const_cast<unsigned char*>((unsigned char*)src->imageData), 150, &markers, &numMarkers) < 0) 
	{
		return false;
	}
	bool foundAny = false;

	mytag.clear();

	float modelViewMatrix_[16];
	for(int m = 0; m < numMarkers; ++m) {
		if(markers[m].id != -1 && markers[m].cf >= 0.75) {
			tracker->calcOpenGLMatrixFromMarker(&markers[m], patternCenter_, patternWidth_, modelViewMatrix_);
			float x = modelViewMatrix_[12] / 1000.0;
			float y = modelViewMatrix_[13] / 1000.0;
			float z = modelViewMatrix_[14] / 1000.0;
			float yaw = -atan2(modelViewMatrix_[1], modelViewMatrix_[0]);
			if (yaw < 0)
			{
				yaw += 6.28;
			}

			if ((x == 0.0 && y == 0.0 && yaw == 0.0) || (x > 10000.0 && y > 10000.0) || (x < -10000.0 && y < -10000.0) || (z <= 0.001))
			{
				// ARTKPlus bug that occurs sometimes
				continue;
			}

			printf("ARtag Info:\n");
			printf("Id: %d\t Conf: %.2f\n", markers[m].id, markers[m].cf);
			printf("x: %.2f \t y: %.2f \t z: %.2f \t yaw: %.2f\n", x,y,z,yaw);
			printf("\n");
			
			foundAny = true;
			
			#if 0
			// no longer putting tag info on image.
			char str[30];
			sprintf(str,"%d",markers[m].id);
			CvFont font1 = cvFont(3,3);
			CvFont font2 = cvFont(1,1);
			cvPutText (dst,str,cvPoint( markers[m].pos[0]+25,markers[m].pos[1]+10),&font1,cvScalar(0,0,255));
			sprintf(str,"(%.2f,%.2f,%.2f)", x*fudge + xoffset, -(y*fudge + yoffset), yaw + yawoffset);
			cvPutText (dst,str,cvPoint( markers[m].pos[0]+25,markers[m].pos[1]+25),&font2,cvScalar(0,0,255));
			#endif

			cv::Mat PoseM(4, 4, CV_32F, modelViewMatrix_);
			cv::transpose(PoseM,PoseM);
			CvMat pose = PoseM;

			// save artag struct for access later
			if (markers[m].id >= 0 && markers[m].id <= ARTAG_MAX_ID && !allStop)
			{
				ARtag mt;
				mt.setId(markers[m].id);
				mt.setPose(&pose);
				mt.setPoseAge(0);
				mt.setCamId(camID);
				mytag.push_back(mt);
			}
		}
	}

	return foundAny;
}
/*! \fn ARtag * ARtagLocalizer::getARtag(int index)
 * 	\brief Get the ARtag detected. Need to know the total number of ARtags in view first.
 * 	\param index the index in the vector of ARtag.
 * 	\return ARtag object that stores information of the id and pose.
 * 	\see ARtag
 * 	\see getARtagSize
 */
ARtag * ARtagLocalizer::getARtag(int index)
{
	return &mytag[index];
}

/*! \fn int ARtagLocalizer::getARtagSize()
 * 	\brief Get te size of the ARtag seen in the view the last time the getARtagPose was called.
 * 	\return the total number of the ARtag that was seen in the view.
 * 	\see getARtagPose
 */
int ARtagLocalizer::getARtagSize()
{
	return mytag.size();
}

/*! \fn void ARtagLocalizer::setARtagOffset(float x_offset, float y_offset, float yaw_offset)
 * 	\brief Set the ARtag pose offset.
 * 	\param x_offset the ARtag pose offset in x 
 * 	\param y_offset the ARtag pose offset in y 
 * 	\param yaw_offset the ARtag pose offset in yaw 
 */
void ARtagLocalizer::setARtagOffset(float x_offset, float y_offset, float yaw_offset)
{
	xoffset = x_offset;
	yoffset = y_offset;
	yawoffset = yaw_offset;
}

/*! \fn int ARtagLocalizer::cleanupARtagPose(void)
 * 	\brief Clean up the stuff for ARtag localizer, should have called in destructor?
 * 	\return 0 on success
 */
int ARtagLocalizer::cleanupARtagPose(void)
{
	delete tracker;
	return 0;
}
