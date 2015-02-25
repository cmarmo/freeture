/*
								Camera.h

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau -- FRIPON-GEOPS-UPSUD
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		21/01/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    Camera.h
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    13/06/2014
* \brief   Acquisition thread.
*/

#pragma once
#include "config.h"

#ifdef WINDOWS
#include <windows.h>
#else
    #ifdef LINUX
    #define BOOST_LOG_DYN_LINK 1
    #endif
#endif

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/core.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include "Frame.h"
#include "TimeDate.h"
#include "Conversion.h"
#include "SaveImg.h"
#include "Fits.h"
#include "Fits2D.h"
#include "Fits3D.h"
#include "ManageFiles.h"
#include "Conversion.h"
#include "ELogSeverityLevel.h"
#include "EImgBitDepth.h"
#include "ECamBitDepth.h"
#include "EAcquisitionMode.h"
#include "StackedFrames.h"
#include "ECamType.h"
//#include "serialize.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <algorithm>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <boost/circular_buffer.hpp>



#include "CameraSDK.h"
#include "CameraSDKPylon.h"
#include "CameraSDKAravis.h"

using namespace boost::filesystem;

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

using namespace cv;
using namespace std;

class Camera{

	private:

        //! Logger.
        src::severity_logger< LogSeverityLevel > log;

		//! Stop flag of the thread.
        bool mustStop;

		//! Mutex on the stop flag.
		boost::mutex mustStopMutex;

        //! Acquisition thread.
		boost::thread *m_thread;

        //! Camera SDK to use.
		CameraSDK *camera;

        //! Camera's exposure value.
		int exposure;

		//! Camera's gain value.
        int gain;

        //! Camera's fps.
        int fps;

        //! Camera's bit depth.
        CamBitDepth bitdepth;

        //! Camera's type (Basler, Dmk...).
        CamType cameraType;

        //! Location of the configuration file.
        string configFile;

        string savePath;

        unsigned int stackCpt;

        unsigned int frameFailedCpt;

        //! Frame counter.
        unsigned int frameCpt;

        int frameToSum;
        int frameToWait;
        bool stackEnabled;

        boost::condition_variable               *c_newElemFrameBuffer;
        boost::mutex                            *m_frameBuffer;
        boost::circular_buffer<Frame>           *frameBuffer;

        boost::circular_buffer<StackedFrames>   *stackedFramesBuffer;
        boost::mutex                            *m_stackedFramesBuffer;
        boost::condition_variable               *c_newElemStackedFramesBuffer;

        bool                                    *newFrameDet;
        boost::mutex                            *m_newFrameDet;
        boost::condition_variable               *c_newFrameDet;

	public:

        Camera( CamType                                 camType,
                int                                     camExp,
                int                                     camGain,
                CamBitDepth                             camDepth,
                int                                     camFPS,
                int                                     imgToSum,
                int                                     imgToWait,
                bool                                    imgStack,
                boost::circular_buffer<Frame>           *cb,
                boost::mutex                            *m_cb,
                boost::condition_variable               *c_newElemCb,
                boost::circular_buffer<StackedFrames>   *stackedFb,
                boost::mutex                            *m_stackedFb,
                boost::condition_variable               *c_newElemStackedFb,
                bool                                    *newFrameForDet,
                boost::mutex                            *m_newFrameForDet,
                boost::condition_variable               *c_newFrameForDet);

		// Used for single acquisition.
        Camera( CamType         camType,
                int             camExp,
                int             camGain,
                CamBitDepth     camDepth);

		Camera( CamType camType);

		~Camera(void);


        //! Print availble detected cameras.
		void	getListCameras();

        //! Get camera's name by its id.
		bool    getDeviceById(int id, string &device);

		//! Select a device by its id or by its name.
		bool	setSelectedDevice(int id, string name);

        //! Select a device by its name.
		bool	setSelectedDevice(string name);

		//! Wait the end of the acquisition thread.
		void	join();

        //! Acquisition thread operations.
		void	operator()();

		//! Stop the acquisition thread.
		void	stopThread();

		//! Start the acquisition thread.
		void	startThread();

		bool    startGrab(int fps);

        //! Prepare the grab of frames.
		bool    startGrab();

        //! Stop the grabbing of frames.
		void    stopGrab();

        //! Grab one frame.
		bool    grabSingleFrame(Mat &frame, string &date);

		bool    grabSingleFrame(Frame &frame, int camID);

        //! Get camera's width sensor.
		int		getCameraWidth();

		//! Get camera's height sensor.
		int		getCameraHeight();

        //! Get camera's minimum available exposure value.
		double	getCameraExpoMin();

		//! Set camera's exposure time.
		bool	setCameraExposureTime(double value);

		//! Set camera's gain.
		bool	setCameraGain(int);

        //! Set camera's fps.
        bool    setCameraFPS(int fps);

        //! Set camera's bit depth.
		bool	setCameraPixelFormat(CamBitDepth depth);

};