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



#include "SquareDetector.hpp"
#include "Tracker.hpp"

static std::string ToFixedLengthName(size_t number, int length = 8)
{
   std::string result = std::to_string(number);
   assert(result.length() <= length);
   std::string prefix = "";
   while (result.length() + prefix.length() < length)prefix += "0";
   assert((prefix + result).length() == length);
   return prefix + result;
}

int main(int argc, char** argv)
{
  
	clock_t begin_time = clock(); // start clock   
	std::vector<std::string> images; //store images directories
   if (argc == 2)
   {
      std::string videoDirectory(argv[1]);
      if (boost::filesystem::is_directory(videoDirectory))
      {
         int imageCount = 0;
         for (boost::filesystem::directory_iterator it(videoDirectory); it != boost::filesystem::directory_iterator(); ++it)imageCount++;
         for (auto i = 1; i <= imageCount; i++)
         {
            images.push_back(std::string(videoDirectory + "\\" + ToFixedLengthName(i) + ".png"));
         }
      }
   }
   else
   {
      getImageList("data.txt", &images);
   }

	int sz = images.size(); //number of images
	
	std::ofstream ptsfile; //used to store points
	ptsfile.open("out.txt",std::ifstream::app);
	std::vector<Tracker *> Trackers; //store the trackers currently use 
	bool plotflag = true; //decide if plot the result
	int label = 0;
	
	//First Frame
	const char * imgname = images[0].c_str();
	std::vector<std::vector<cv::Point> > squares;
	cv::Mat image = cv::imread(imgname,1);

   SquareDetector fc(image.cols,image.rows); //square detector

	vpImage<unsigned char> I;
	vpImageIo::read(I,imgname);
	
	fc.findSquares(image, squares);
	if(plotflag) {fc.drawSquares(image,squares, 1);}
	std::vector<std::vector<cv::Point> > cvpts; //current stored coordinates

	if(!squares.empty())
	{
		for(int i = 0; i<squares.size(); i++)
		{
			pad(squares[i],5);
			++label;
			std::vector<vpImagePoint> vispCorners;
			for(int j = 0; j<4; j++)
			{
				vispCorners.push_back(vpImagePoint(squares[i][j].y,squares[i][j].x));
			}

			Tracker * temp = new Tracker(I,vispCorners,label);
			Trackers.push_back(temp);
			Trackers[i]->start(I);
			Trackers[i]->check_pts(cvpts);
			Trackers[i]->write_pts(ptsfile,images[0]);			
		}
	}

	if(plotflag) {showResult(imgname,cvpts);}

	
	
	
	//Start from the second frame
	for(int cnt = 1; cnt < sz; cnt++)
	{
		std::cout<<images[cnt]<<std::endl;
		
		//read image
		const char * imgname = images[cnt].c_str();
		std::vector<std::vector<cv::Point> > squares;
		cv::Mat image = cv::imread(imgname,1);
		vpImage<unsigned char> I;
		vpImageIo::read(I,imgname);
		
		// temporary stored vector
		std::vector<std::vector<cv::Point> > cvpts_temp;
		cvpts_temp.clear();

		//find contours and pad contours
		fc.findSquares(image, squares);
		for(int i = 0; i<squares.size(); i++)
		{
			pad(squares[i],5);
		}
		if(plotflag) {fc.drawSquares(image,squares, cnt); cv::waitKey(10);}
		
		
		//recover failed Trackers
		for(int i = 0; i<Trackers.size();)
		{
			bool del_flag = false;
			int delLabel = Trackers[i]->getID();
			
			//ptsfile,images[cnt],cvpts_temp
			//check if need deleting
			if(Trackers[i]->start(I))
			{
				if(!(Trackers[i]->check_pts(cvpts_temp)))
				{
					del_flag = true;
				}
				else
				{	
					if(!fc.checkEqual(cvpts_temp.back(),cvpts[i]) || fc.maxDist(cvpts_temp.back(),cvpts[i])>30)
					{
						cvpts_temp.pop_back();
						del_flag = true;
					}
				}
			}
			else
			{
				del_flag = true;
			}

			if(del_flag)
			{
				bool recover = false;
				Trackers.erase(Trackers.begin()+i);
				for(int j = 0; j<squares.size(); j++)
				{
					if(checkEqual(cvpts[i],squares[j]))
					{
						recover = true;
						std::cout<<"Renew!!"<<std::endl;
						std::vector<vpImagePoint> vispCorners;
						for(int m = 0; m<4; m++)
						{
							vispCorners.push_back(vpImagePoint(squares[j][m].y,squares[j][m].x));
						}
						Tracker * temp = new Tracker(I,vispCorners,delLabel);
						Trackers.insert(Trackers.begin()+i,temp);
						Trackers[i]->start(I);
						Trackers[i]->check_pts(cvpts_temp);
						Trackers[i]->write_pts(ptsfile,images[cnt]);
						std::cout<<Trackers[i]->getID()<<" ";	
						//std::cout<<Trackers[i]->getID()<<" ";	
						squares.erase(squares.begin()+j);//remove squares[j] from square
						++i;
						break;
					}
				}
				if(!recover)
				{
					cvpts.erase(cvpts.begin()+i);
				}
				continue;
			}
			Trackers[i]->write_pts(ptsfile,images[cnt]);
			std::cout<<Trackers[i]->getID()<<" ";
			++i;
		}



		//add new rectangles
		for(int i = 0; i<squares.size();i++)
		{
			bool add_flag = true;
			for(int j = 0; j<cvpts_temp.size(); j++)
			{
				if(fc.isOverlap(cvpts_temp[j],squares[i]))
				{
					add_flag = false;
					break;
				}
			}
			if(add_flag)
			{
				++label;
				std::vector<vpImagePoint> vispCorners;
				for(int j = 0; j<4; j++)
				{
					vispCorners.push_back(vpImagePoint(squares[i][j].y,squares[i][j].x));
				}

				Tracker * temp = new Tracker(I,vispCorners,label);
				Trackers.push_back(temp);
				Trackers.back()->start(I);
				Trackers.back()->check_pts(cvpts_temp);	
				Trackers.back()->write_pts(ptsfile,images[cnt]);
				std::cout<<Trackers.back()->getID()<<" ";	
			}
		}
		
		std::cout<<std::endl;
		
		if(plotflag) {showResult(imgname,cvpts_temp); cv::waitKey(10);}
		//update cvpts
		cvpts.clear();
		cvpts = cvpts_temp;
	


	


		
	}	
	std::cout<< float( clock() - begin_time) /CLOCKS_PER_SEC<<std::endl; //end clock

	return 0;
}
