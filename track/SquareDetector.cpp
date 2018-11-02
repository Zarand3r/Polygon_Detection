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

const char * wndname = "Square Detection";

SquareDetector::SquareDetector(int width,int height)
   :WIDTH(width),HEIGHT(height)
{
   //WIDTH = 1242;
   //HEIGHT = 375;
   edge_margin = 10;
   length_ratio = 10;
   ang_dif = 25;
   ang_err = 45;
   minArea = 400;
   maxAreaRatio = 0.05;
   special = false;
}


//helper functions used for checking line segments intersection
bool SquareDetector::intersection(cv::Point & o1, cv::Point & p1, cv::Point & o2, cv::Point & p2, cv::Point &r)
{
   cv::Point x = o2 - o1;
   cv::Point d1 = p1 - o1;
   cv::Point d2 = p2 - o2;

   float cross = d1.x*d2.y - d1.y*d2.x;
   if (std::abs(cross)<1e-8)
      return false;
   double t1 = (x.x * d2.y - x.y * d2.x) / cross;
   r = o1 + d1 * t1;
   return true;
}

bool SquareDetector::onSegment(cv::Point & p, cv::Point & q, cv::Point & r)
{
   if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
      q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
      return true;
}

int SquareDetector::orientation(cv::Point p, cv::Point q, cv::Point r)
{
   int val = (q.y - p.y) * (r.x - q.x) -
      (q.x - p.x) * (r.y - q.y);

   if (val == 0) return 0;  // colinear

   return (val > 0) ? 1 : 2; // clock or counterclock wise
}

bool SquareDetector::doIntersect(cv::Point p1, cv::Point q1, cv::Point p2, cv::Point q2)
{
   int o1 = orientation(p1, q1, p2);
   int o2 = orientation(p1, q1, q2);
   int o3 = orientation(p2, q2, p1);
   int o4 = orientation(p2, q2, q1);

   // General case
   if (o1 != o2 && o3 != o4)
      return true;

   // Special Cases
   // p1, q1 and p2 are colinear and p2 lies on segment p1q1
   if (o1 == 0 && onSegment(p1, p2, q1)) return true;

   // p1, q1 and p2 are colinear and q2 lies on segment p1q1
   if (o2 == 0 && onSegment(p1, q2, q1)) return true;

   // p2, q2 and p1 are colinear and p1 lies on segment p2q2
   if (o3 == 0 && onSegment(p2, p1, q2)) return true;

   // p2, q2 and q1 are colinear and q1 lies on segment p2q2
   if (o4 == 0 && onSegment(p2, q1, q2)) return true;

   return false; // Doesn't fall in any of the above cases
}


// helper function used for checking if a point is inside a convex polygon


// check a point is to the left(-1), right(+1) or on the line segment(0)
int SquareDetector::checkSide(const cv::Point & p1, const cv::Point & p2, const cv::Point & p)
{
   double val = (p.y - p1.y)*(p2.x - p1.x) - (p.x - p1.x)*(p2.y - p1.y);
   if (std::abs(val) < 1e-6) return 0;
   else if (val < 0) return -1;
   else return 1;
}


bool SquareDetector::insidePolygon(const std::vector<cv::Point> & corners, const cv::Point & p)
{
   int a = checkSide(corners[0], corners[1], p);
   int b = checkSide(corners[1], corners[2], p);
   int c = checkSide(corners[2], corners[3], p);
   int d = checkSide(corners[3], corners[0], p);
   if (a == 0 || b == 0 || c == 0 || d == 0) return true;
   else if (a == b && b == c && c == d) return true;
   else return false;
}


bool SquareDetector::isOverlap(std::vector<cv::Point> & corners1, std::vector<cv::Point> & corners2)
{

   bool failsafe = cv::norm(corners1[0] - corners2[0])<10 && cv::norm(corners1[1] - corners2[1])<10 && cv::norm(corners1[2] - corners2[2])<10 && cv::norm(corners1[3] - corners2[3])<10;
   bool corner = insidePolygon(corners1, corners2[0]) || insidePolygon(corners1, corners2[1]) || insidePolygon(corners1, corners2[2]) || insidePolygon(corners1, corners2[3]) || insidePolygon(corners2, corners1[0]) || insidePolygon(corners2, corners1[1]) || insidePolygon(corners2, corners1[2]) || insidePolygon(corners2, corners1[3]);
   bool side = false;
   for (int i = 0; i<4; i++)
   {
      for (int j = 0; j<4; j++)
      {
         if (doIntersect(corners1[j], corners1[(j + 1) % 4], corners2[i], corners2[(i + 1) % 4]))
         {
            side = true;
            break;
         }
      }
   }

   return failsafe || corner || side;


}


double SquareDetector::countOverlap(std::vector<cv::Point> & corners1, std::vector<cv::Point> & corners2)
{
   std::vector<cv::Point> cpts; //crossing points

   for (int i = 0; i<4; i++)
   {
      if (insidePolygon(corners1, corners2[i])) cpts.push_back(corners2[i]);
      if (insidePolygon(corners2, corners1[i])) cpts.push_back(corners1[i]);
   }
   cv::Point temp;
   for (int i = 0; i<4; i++)
   {
      for (int j = 0; j<4; j++)
      {
         if (doIntersect(corners1[j], corners1[(j + 1) % 4], corners2[i], corners2[(i + 1) % 4]))
         {
            intersection(corners1[j], corners1[(j + 1) % 4], corners2[i], corners2[(i + 1) % 4], temp);
            cpts.push_back(temp);
         }
      }
   }
   return std::abs(cv::contourArea(cv::Mat(cpts)));
}


// helper functions:
// finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
double SquareDetector::angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
   double dx1 = pt1.x - pt0.x;
   double dy1 = pt1.y - pt0.y;
   double dx2 = pt2.x - pt0.x;
   double dy2 = pt2.y - pt0.y;
   return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// helper function:
// comparision function object
static bool compareContourAreas(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2)
{
   double i = std::abs(cv::contourArea(cv::Mat(contour1)));
   double j = std::abs(cv::contourArea(cv::Mat(contour2)));
   return (i>j);
}

// sort corners of polygon return true if success
bool SquareDetector::sortCorners(std::vector<cv::Point> & corners)
{
   cv::Point center(0, 0);
   for (int i = 0; i<4; i++)
      center += corners[i];

   center *= (1.0 / 4.0);
   std::vector <cv::Point> top, bot;
   for (int i = 0; i < 4; i++)
   {
      if (corners[i].y < center.y)
         top.push_back(corners[i]);
      else
         bot.push_back(corners[i]);
   }
   if (top.size() != 2) return false;

   cv::Point tl = top[0].x > top[1].x ? top[1] : top[0];
   cv::Point tr = top[0].x > top[1].x ? top[0] : top[1];
   cv::Point bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
   cv::Point br = bot[0].x > bot[1].x ? bot[0] : bot[1];

   corners.clear();
   corners.push_back(tl);
   corners.push_back(tr);
   corners.push_back(br);
   corners.push_back(bl);
   return true;
}

// calculate the angle defined by two Points wrt horizontal line
double SquareDetector::calAngle(const cv::Point & p1, const cv::Point & p2)
{
   int dx = p2.x - p1.x;
   int dy = p2.y - p1.y;
   return std::atan(double(dy) / double(dx))*180.0 / 3.14159265;
}

// calculate the angle defined by two Points wrt vertical line
double SquareDetector::calAngle2(const cv::Point & p1, const cv::Point & p2)
{
   int dx = p2.x - p1.x;
   int dy = p2.y - p1.y;
   return std::atan(double(dx) / double(dy))*180.0 / 3.14159265;
}

// calculate average dist between two set of corner points
double SquareDetector::cornerDist(std::vector<cv::Point>& pts1, std::vector<cv::Point>& pts2)
{
   return (cv::norm(pts1[0] - pts2[0]) + cv::norm(pts1[1] - pts2[1]) + cv::norm(pts1[2] - pts2[2]) + cv::norm(pts1[3] - pts2[3])) / 4.0;
}

// compare two rectangle and return true if the first one is more like a rectangle
bool SquareDetector::compareRectangles(std::vector<cv::Point>& pts1, std::vector<cv::Point>& pts2)
{
   //double l1deg1 = calAngle(pts1[0],pts1[1]);
   //double l2deg1 = calAngle2(pts1[1],pts1[2]);
   //double l3deg1 = calAngle(pts1[3],pts1[2]);
   //double l4deg1 = calAngle2(pts1[0],pts1[3]);
   //double difdeg1 = abs(l1deg1-l3deg1)+abs(l2deg1-l4deg1);

   //double l1deg2 = calAngle(pts2[0],pts2[1]);
   //double l2deg2 = calAngle2(pts2[1],pts2[2]);
   //double l3deg2 = calAngle(pts2[3],pts2[2]);
   //double l4deg2 = calAngle2(pts2[0],pts2[3]);
   //double difdeg2 = abs(l1deg2-l3deg2)+abs(l2deg2-l4deg2);

   return fabs(cv::contourArea(cv::Mat(pts1)))<fabs(cv::contourArea(cv::Mat(pts2)));
}

// check two rectangles if they are approximately equal 
bool SquareDetector::checkEqual(std::vector<cv::Point>& pts1, std::vector<cv::Point>& pts2)
{
   float a1 = fabs(cv::contourArea(cv::Mat(pts1)));
   float a2 = fabs(cv::contourArea(cv::Mat(pts2)));
   return ((a1 <= a2 ? a1 : a2) / (a1>a2 ? a1 : a2))>0.7;
}

// check if the length and angles of lines are OK
bool SquareDetector::checkSquare(std::vector<cv::Point> & corners)
{
   double l1 = cv::norm(corners[0] - corners[1]);
   double l2 = cv::norm(corners[1] - corners[2]);
   double l3 = cv::norm(corners[2] - corners[3]);
   double l4 = cv::norm(corners[3] - corners[0]);
   double len[4] = { l1,l2,l3,l4 };
   double minlen = *(std::min_element(len, len + 4));
   double maxlen = *(std::max_element(len, len + 4));
   if (maxlen / minlen>length_ratio) return false;

   double l1deg = calAngle(corners[0], corners[1]);
   double l2deg = calAngle2(corners[1], corners[2]);
   double l3deg = calAngle(corners[3], corners[2]);
   double l4deg = calAngle2(corners[0], corners[3]);

   if (std::abs(l1deg - l3deg)>ang_dif || std::abs(l2deg - l4deg)>ang_dif) return false;

   if (std::abs(l2deg)>ang_err || std::abs(l4deg)>ang_err) return false;

   if (special && (l2<1.3*l1 || l4<1.3*l1 || l2<1.3*l3 || l4<1.3*l1)) return false;

   return true;
}

//find the max distance between corners of two rectangles
double SquareDetector::maxDist(std::vector<cv::Point>& pts1, std::vector<cv::Point>& pts2)
{
   double max = cv::norm(pts1[0] - pts2[0]);
   for (int i = 1; i<4; i++)
   {
      if (cv::norm(pts1[i] - pts2[i])>max)
         max = norm(pts1[i] - pts2[i]);
   }
   return max;
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void SquareDetector::findSquares(const cv::Mat& image, std::vector<std::vector<cv::Point> >& squares)
{
   int thresh = 50, N = 11;
   cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;
   std::vector<cv::Mat> channels;
   cv::split(image, channels);
   //dilate(image, timg, cv::Mat(), cv::Point(-1, -1));
   // then blur it so that the ocean/sea become one big segment to avoid detecting them as 2 big squares.
   //medianBlur(timg, timg, 3);
   // down-scale and upscale the image to filter out the noise
   //cv::pyrDown(timg, pyr, cv::Size(image.cols/2, image.rows/2));
   //cv::pyrUp(pyr, timg, image.size());
   std::vector<std::vector<cv::Point> > contours;
   timg = image.clone();
   //auto clahe = cv::createCLAHE();
   // find squares in every color plane of the image
   for (int c = 0; c < 3; c++)
   {
      int ch[] = { c, 0 };
      cv::mixChannels(&timg, 1, &gray0, 1, ch, 1);
      //cv::equalizeHist(gray0, gray0);
      //clahe->apply(gray0, gray0);
      cv::GaussianBlur(gray0, gray0, cv::Size(3, 3), 0, 0);
      //dilate(gray0, gray0, cv::Mat(), cv::Point(-1, -1));
      //cv::pyrDown(gray0, pyr, cv::Size(image.cols/2, image.rows/2));
      //cv::pyrUp(pyr, gray0, image.size());
      // try several threshold levels
      for (int l = 0; l < N; l++)
      {
         // hack: use Canny instead of zero threshold level.
         // Canny helps to catch squares with gradient shading
         if (l == 0)
         {
            // apply Canny. Take the upper threshold from slider
            // and set the lower to 0 (which forces edges merging)
            cv::Canny(gray0, gray, 0, thresh, 5, true);
            //dilate(gray, gray,cv::Mat(), cv::Point(-1, -1));
         }
         else
         {
            // apply threshold if l!=0:
            //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
            gray = gray0 >= (l + 1) * 255 / N;
         }

         // find contours and store them all as a list
         cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
         //cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_TC89_L1);
         //findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS);
         //findContours(gray, contours, CV_RETR_LIST, CHAIN_APPROX_NONE);

         std::vector<cv::Point> approx;

         // test each contour
         for (size_t i = 0; i < contours.size(); i++)
         {
            // approximate contour with accuracy proportional
            // to the contour perimeter
            cv::approxPolyDP(cv::Mat(contours[i]), approx, std::min(cv::arcLength(cv::Mat(contours[i]), true)*0.03, 15.0), true);

            //approxPolyDP(cv::Mat(contours[i]), approx, 10.0, true);

            // square contours should have 4 vertices after approximation
            // relatively large area (to filter out noisy contours)
            // and be convex.
            if (approx.size() == 4 &&
               fabs(cv::contourArea(cv::Mat(approx))) > minArea &&
               fabs(cv::contourArea(cv::Mat(approx))) < maxAreaRatio*WIDTH*HEIGHT &&
               cv::isContourConvex(cv::Mat(approx)))
               squares.push_back(approx);
         }
      }
   }
   //make sure the rectangles are within the border of image and sort corners for future use
   for (std::vector<std::vector<cv::Point> >::iterator con = squares.begin(); con != squares.end();)
   {
      int xcoor[4] = { (*con)[0].x,(*con)[1].x,(*con)[2].x,(*con)[3].x };
      int ycoor[4] = { (*con)[0].y,(*con)[1].y,(*con)[2].y,(*con)[3].y };
      int minx = *(std::min_element(xcoor, xcoor + 4));
      int maxx = *(std::max_element(xcoor, xcoor + 4));
      int miny = *(std::min_element(ycoor, ycoor + 4));
      int maxy = *(std::max_element(ycoor, ycoor + 4));
      if (minx <= 1 + edge_margin || maxx >= WIDTH - edge_margin || miny <= 1 + edge_margin || maxy >= HEIGHT - edge_margin || (!sortCorners(*con)))
      {
         con = squares.erase(con);
         continue;
      }
      con++;
   }

   //sort contours by areas in descending order

   std::sort(squares.begin(), squares.end(), compareContourAreas);


   //make sure the length ratio and angle ratio is not too big and deal with overlapping 
   // and remove overlap
   for (std::vector<std::vector<cv::Point> >::iterator con = squares.begin(); con != squares.end();)
   {
      bool delflag = false;
      if (!checkSquare(*con))
      {
         con = squares.erase(con);
         continue;
      }

      for (std::vector<std::vector<cv::Point> >::iterator con2 = squares.begin(); con2 != con; con2++)
      {
         if (isOverlap(*con, *con2))
         {
            delflag = true;
            if (countOverlap(*con, *con2) / fabs(cv::contourArea(cv::Mat(*con)))>0.8)
            {
               if (!compareRectangles(*con2, *con) && checkEqual(*con2, *con))
               {
                  std::swap(*con2, *con);
                  break;
               }
            }
         }
      }
      if (delflag)
      {
         con = squares.erase(con);
         continue;
      }
      con++;
   }
   // make sure no overlap
   for (std::vector<std::vector<cv::Point> >::iterator con = squares.begin(); con != squares.end();)
   {
      bool delflag = false;
      for (std::vector<std::vector<cv::Point> >::iterator con2 = squares.begin(); con2 != con; con2++)
      {
         if (isOverlap(*con, *con2))
         {
            delflag = true;
         }
      }
      if (delflag)
      {
         con = squares.erase(con);
         continue;
      }
      con++;
   }

   //cv::imshow(wndname,mask);
   //cv::waitKey(0);
}


// the function draws all the squares in the image
void SquareDetector::drawSquares(cv::Mat& image, const std::vector<std::vector<cv::Point> >& squares, int count)
{
   for (size_t i = 0; i < squares.size(); i++)
   {
      const cv::Point* p = &squares[i][0];
      int n = (int)squares[i].size();
      cv::polylines(image, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3, CV_AA);
   }
   cv::imshow("window", image);
   cv::imwrite("processed/square/" + std::to_string(count) + ".png", image);

}

void SquareDetector::setMinArea(double val)
{
   minArea = val;
}

void SquareDetector::setMaxArea(double val)
{
   maxAreaRatio = val;
}

void SquareDetector::setSpecial(bool val)
{
   special = val;
}

//int main(int argc, char** argv)
//{
//	clock_t begin_time = clock();
//    cv::namedWindow( wndname, 1 );
//    std::vector<std::vector<cv::Point> > squares;
//        cv::Mat image = cv::imread(imgname, 1);		
//        findSquares(image, squares);
//        drawSquares(image, squares);
//		std::cout<< float( clock() - begin_time) /CLOCKS_PER_SEC<<std::endl;
//        int c = cv::waitKey(0);
//		return 0;
//}
