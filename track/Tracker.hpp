'''MIT License

Copyright (c) 2018 Shahrouz Ryan Alimo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
Authors: Shahrouz Ryan Alimo 
Modified: March. 2018

'''


#ifndef TRACKER
#define TRACKER

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <visp/vpImageIo.h>
#include <visp/vpDisplayGDI.h>
#include <visp/vpDisplayX.h>
#include <visp/vpDisplayOpenCV.h>
#include <visp/vpTemplateTrackerSSDInverseCompositional.h>
#include <visp/vpTemplateTrackerZNCCInverseCompositional.h>
#include <visp/vpTemplateTrackerWarpHomography.h>
#ifdef WIN32
#include "io.h" 
#else
#include <unistd.h>
#endif // WIN32
#include <sstream>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/filesystem.hpp>

// default : HEIGHT 1200 / WIDTH 1600
#define HEIGHT 1200
#define WIDTH 1600

class Tracker{
	private:
		vpTemplateTrackerWarpHomography * warp;
		vpTemplateTrackerSSDInverseCompositional * tracker;
		vpTemplateTrackerZone * zone_ref;
		vpTemplateTrackerZone * zone_warped;
		int id;
	public:
		vpImagePoint rec_corners[4];
		Tracker(vpImage<unsigned char> & I, std::vector<vpImagePoint> & points, int initID);
		~Tracker();
		bool start(vpImage<unsigned char> & I);
		bool check_pts(std::vector<std::vector<cv::Point> >  & cvpts);
		void write_pts(std::ofstream & ptsfile , std::string & number);
		int getID();
};

void pad( std::vector<cv::Point>& con, int N, cv::Size sz = cv::Size(WIDTH, HEIGHT));

void getImageList( std::string filename,  std::vector<std::string>* il );

void showResult(const char * name, const std::vector<std::vector<cv::Point> > & cvpts);

bool checkEqual(std::vector<cv::Point> & square1, std::vector<cv::Point> & square2);

#endif
