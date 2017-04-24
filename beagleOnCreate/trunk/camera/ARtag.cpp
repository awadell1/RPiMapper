#include "ARtag.h"

/*! \file ARtag.cpp
 */

/**
 * 	\class ARtag ARtag.h "ARtag.h"
 *	\brief ARtag class which holds information like tag ID and pose matrix for the ARtag.
 */

/*! \fn ARtag::ARtag()  : id_(-1), pose_age_(0)
 *  \brief A constructor for ARtag.
 * 	In the constructor, the pose_ CvMat object is initialized.
 */
ARtag::ARtag()  : id_(-1), pose_age_(0)
{
	pose_ = cvCreateMat( 4, 4, CV_32F);
}

/*! \fn ARtag::~ARtag()
 * 	\brief A destructor for ARtag.
 * 	Nothing is done in the destructor.
 */
ARtag::~ARtag()
{
}

/*!	\fn void ARtag::setId(int id)
 *	\brief The setter for ARtag ID.
 *	\param id an interger argument for setting the ARtag ID.
 */
void ARtag::setId(int id)
{
	id_ = id;
}

/*!	\fn unsigned int ARtag::getId() const
 *	\brief The getter for ARtag ID.
 *	\return The unsigned int of ARtag ID.
 */
unsigned int ARtag::getId() const
{
	return id_;
}

/*!	\fn void ARtag::setPose(CvMat * pose)
 *	\brief The setter for pose.
 *	\param pose CvMat object that contains pose.
 */
void ARtag::setPose(CvMat * pose)
{
	for (int i = 0; i < pose->rows; ++i)
		for (int j = 0; j < pose->cols; ++j)
			CV_MAT_ELEM(*pose_ , float, i, j) = CV_MAT_ELEM(*pose , float, i, j);
}

/*!	\fn CvMat * ARtag::getPose() const
 * 	\brief The getter for pose.
 * 	\return The pose of the ARtag of type CvMat*
 */
CvMat * ARtag::getPose() const
{
	return pose_;
}

/*!	\fn void ARtag::setPoseAge(unsigned int age)
 *	\brief The setter for pose age. Used for timestamp.
 *	\param age an unsigned int input argument for age of the pose.
 */
void ARtag::setPoseAge(unsigned int age)
{
	pose_age_ = age;
}

/*!	\fn unsigned int ARtag::getPoseAge() const
 * 	\brief The getter for pose age.
 * 	\return unsigned int type of pose age.
 */
unsigned int ARtag::getPoseAge() const
{
	return pose_age_;
}

/*! \fn void ARtag::setCamId(int id)
 * 	\brief Set the camera ID associated with this ARtag object.
 * 	\param id the id of camera in case there are multiple camera.
 */
void ARtag::setCamId(int id)
{
	cam_id_ = id;
}

/*!	\fn unsigned int ARtag::getCamId() const
 * 	\brief Get the camera ID associated with this ARtag object.
 * 	\return a unsigned int of camera ID
 */
unsigned int ARtag::getCamId() const
{
	return cam_id_;
}