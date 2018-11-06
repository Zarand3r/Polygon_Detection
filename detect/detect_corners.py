# Created by Richard Bao on 10/20/18.
# Copyright © 2018 Richard Bao, California Institute of Technology. All rights reserved.

import cv2
import numpy as np
import matplotlib.pyplot as plt


#=================================================================================# 
# Utilities 
#=================================================================================# 

def drawCorners(filename, corners, output, isPath = False, colors = (0,255,0)):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
	for item in corners:
		x, y = item
		cv2.circle(img, (x,y), 5, colors, -1)
	cv2.imwrite(output,img)

#=================================================================================# 
# Corner Detector
#=================================================================================# 

def harris_corners(filename, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	dst = cv2.cornerHarris(img,2,3,0.04)
	dst = cv2.dilate(dst,None, iterations = 3)

	img[dst>0.01*dst.max()] = [0,255,0]

	plt.imshow(img,cmap = 'gray')
	plt.title('Corners'), plt.xticks([]), plt.yticks([])

	plt.show()

def max_harris_corners(filename, output_directory, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	# find Harris corners
	dst = cv2.cornerHarris(img,2,3,0.04)
	dst = cv2.dilate(dst,None)
	ret, dst = cv2.threshold(dst,0.01*dst.max(),255,0)
	dst = np.uint8(dst)

	# find centroids
	ret, labels, stats, centroids = cv2.connectedComponentsWithStats(dst)

	# define the criteria to stop and refine the corners
	criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 0.001)
	corners = cv2.cornerSubPix(gray,np.float32(centroids),(5,5),(-1,-1),criteria)
	drawCorners(img, corners, output = output_directory+"harris_corners.jpg")

	return corners

def shitomasi(filename, output_directory, number = 10, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	corners = cv2.goodFeaturesToTrack(img, number, 0.05, 25)
	corners = np.float32(corners)

	good_corners = np.squeeze(corners, axis = 1)
	drawCorners(img, good_corners, output = output_directory+"good_corners.jpg")

	return good_corners

def merge_corners():
	filename = 'test/test4.png'
	img = cv2.imread(filename)
	corners1 = shitomasi(img)
	corners2 = max_harris_corners(img)
	for item in corners1:
		x, y = item
		cv2.circle(img, (x,y), 5, (0,255,0), -1)
	for item in corners2:
		x, y = item
		cv2.circle(img, (x,y), 5, (255,0,0), -1)

	cv2.imshow("Top corners", img)
	cv2.waitKey()



