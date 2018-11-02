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


#include "Tracker.hpp"

//Constructor
Tracker::Tracker(vpImage<unsigned char> & I, std::vector<vpImagePoint> & points, int initID)
{
	id = initID;
    warp = new vpTemplateTrackerWarpHomography;
    tracker = new vpTemplateTrackerSSDInverseCompositional(warp);
    zone_ref = new vpTemplateTrackerZone;
    zone_warped = new vpTemplateTrackerZone;
    tracker->setSampling(1,1);
    tracker->setLambda(1.2);
    tracker->setIterationMax(10000);//100000
    tracker->setThresholdRMS(1e-9);
    tracker->setPyramidal(3,1);
	tracker->initFromPoints(I,points,true);
    //tracker->initClick(I,true);
    *zone_ref = tracker->getZoneRef();
}
	
//Destructor
Tracker::~Tracker()
{
 	delete warp;
   	delete tracker;	
	delete zone_ref;
	delete zone_warped;
}

//Start Tracking return false if fails
bool Tracker::start(vpImage<unsigned char> & I)
{
	try{
		tracker->track(I);
	}
	catch ( vpTrackingException &ExceptObj ) 
	{		/* handle exception: */
    	std::cerr << ExceptObj.what() << std::endl;
        return false;
    }
	catch(...)
	{
		std::cerr << "Unspecified Error" << std::endl;
		return false;
	}
	//tracker->display(I);
	vpColVector p = tracker->getp();
	warp->warpZone(*zone_ref,p,*zone_warped);
	return true;
}	
	
//Show pts coordinates, return false if  out of bound
bool Tracker::check_pts(std::vector<std::vector<cv::Point> >  & cvpts){

	vpTemplateTrackerTriangle triangle;
	zone_warped->getTriangle(0, triangle);
	std::vector<vpImagePoint> corners1;
	triangle.getCorners( corners1 );
	rec_corners[0] = corners1[0];
	rec_corners[1] = corners1[1];
	rec_corners[2] = corners1[2];

	vpTemplateTrackerTriangle triangle2;
	zone_warped->getTriangle(1, triangle2);
	std::vector<vpImagePoint> corners2;
	triangle2.getCorners( corners2 );
	rec_corners[3] = corners2[1];
	
	for (int i = 0;i<4;i++){
		if(
		rec_corners[i].get_u()>=WIDTH ||
		rec_corners[i].get_u()<=0    ||
		rec_corners[i].get_v()>=HEIGHT ||
		rec_corners[i].get_v()<=0
		)
		{
			return false;
		}
	}

	cvpts.push_back(std::vector<cv::Point>());
	int n = cvpts.size()-1;
	cvpts[n].push_back(cv::Point(rec_corners[0].get_u(),rec_corners[0].get_v()));
	cvpts[n].push_back(cv::Point(rec_corners[1].get_u(),rec_corners[1].get_v()));
	cvpts[n].push_back(cv::Point(rec_corners[2].get_u(),rec_corners[2].get_v()));
	cvpts[n].push_back(cv::Point(rec_corners[3].get_u(),rec_corners[3].get_v()));
	
	return true;
}

void Tracker::write_pts(std::ofstream & ptsfile , std::string & number)
{	
	//std::cout<<number.substr(51,10)<<","<<rec_corners[0]<<","<<rec_corners[1]<<","<<rec_corners[2]<<","<<rec_corners[3]<<","<<id<<std::endl;

	ptsfile<<number.substr(60,9)<<" , "<<rec_corners[0]<<" , "<<rec_corners[1]<<" , "<<rec_corners[2]<<" , "<<rec_corners[3]<<" , "<<id<<std::endl;
}


int Tracker::getID()
{
	return id;
}



// pad a contour of quadrilateral
void pad( std::vector<cv::Point>& con, int N, cv::Size sz)
{
	cv::Rect br;
    cv::Vec2i center;
    cv::Vec2f unit;

    if( con.size()!=4 )
        std::cerr << "Padded contour not a quadrilateral." << std::endl;

    // Create a rect the size of the frame to prevent extending outside of frame.
    // Bound the contour by a rect, then shift and expand to pad by N.
    br = cv::boundingRect( con );
    center = cv::Vec2i( br.tl() ) + cv::Vec2i(br.width/2, br.height/2);
    for( std::vector<cv::Point>::iterator pt=con.begin();
            pt!=con.end(); ++pt )
    {
        unit = cv::Vec2i(*pt)-center;
        unit = (1/norm(unit)) * unit;

       *pt += cv::Point(N * unit);
    }
    return;
}

// read images directory from the txt file
void getImageList( std::string filename,  std::vector<std::string>* il )
{
	std::string    ifs_file_name = filename;         /* input  file name */
    std::ifstream  ifs;                              /* create ifstream object */

    ifs.open (  ifs_file_name.c_str( ) );         /* open ifstream */
    if ( !ifs ) {
        std::cerr << "\nERROR : failed to open input  file " << ifs_file_name << std::endl;
        exit ( EXIT_FAILURE );
    }
    std::string line;
    while(  getline( ifs,  line,  '\n') )
    {
        il->push_back( line );
    }
    ifs.close ( );                                 /* close ifstream */
}


// show the tracking results
void showResult(const char * name, const std::vector<std::vector<cv::Point> > & cvpts)
{
	cv::Mat image;
	image = cv::imread(name, CV_LOAD_IMAGE_COLOR);
	std::string dir = "processed/tracker/";
	boost::filesystem::path p(name);
	std::string fname = p.filename().string();
	std::string temp_name = dir + fname;
	const char *save_name = temp_name.c_str();
	
	for(int i=0; i<cvpts.size();i++){
		cv::line(image,cvpts[i][0],cvpts[i][1],cv::Scalar( 0, 0, 255 ),2);
		cv::line(image,cvpts[i][1],cvpts[i][2],cv::Scalar( 0, 0, 255 ),2);
		cv::line(image,cvpts[i][2],cvpts[i][3],cv::Scalar( 0, 0, 255 ),2);
		cv::line(image,cvpts[i][0],cvpts[i][3],cv::Scalar( 0, 0, 255 ),2);
	}

	
	//cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
	//cv::imshow("Display window",image);
	cv::imwrite(save_name, image);
}



bool checkEqual(std::vector<cv::Point> & square1, std::vector<cv::Point> & square2)
{
	double thresh = 50.0;
	return cv::norm(square1[0]-square2[0])<thresh && cv::norm(square1[1]-square2[1])<thresh && cv::norm(square1[2]-square2[2])<thresh && cv::norm(square1[3]-square2[3])<thresh;
}


















