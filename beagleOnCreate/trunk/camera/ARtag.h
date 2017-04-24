#ifndef ARTAG_H
#define ARTAG_H 
#include "cv.h"
#include "highgui.h"

/*! \file ARtag.h
 */

class ARtag
{
	public:
        
		ARtag();
		virtual ~ARtag();

		void setId(int id);
		unsigned int getId() const;
   
		void setPose(CvMat * pose);
		CvMat * getPose() const;

		void setPoseAge(unsigned int age);
		unsigned int getPoseAge() const;

		void setCamId(int id);
		unsigned int getCamId() const;

	protected:
		/*! ID of the ARtag. */
		unsigned int id_;
		/*! pose matrix of the ARtag. */
		CvMat * pose_;
		/*! The timestamp of the pose. */
		unsigned int pose_age_;
		/*! Camera ID where this ARtag is detected. */
		unsigned int cam_id_;
};

#endif
