# Created by Richard Bao on 10/20/18.
# Copyright © 2018 Richard Bao, California Institute of Technology. All rights reserved.

import cv2
import numpy as np
import math
from matplotlib import pyplot as plt

#=================================================================================# 
# Utilities 
#=================================================================================# 
def unpack(line):
	for x1,y1,x2,y2 in line:
		return ((x1,y1),(x2,y2))

def drawLines(filename, output, lines, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	for line in lines:
		(x1,y1),(x2,y2) = unpack(line)
		cv2.line(img,(x1,y1),(x2,y2),(0,255,0),6)
	cv2.imwrite(output,img)

def drawLines(filename, output, lines, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
	for line in lines:
		(x1,y1),(x2,y2) = unpack(line)
		cv2.line(img,(x1,y1),(x2,y2),(0,255,0),6)
	cv2.imwrite(output,img)

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
# Hough Lines 
#=================================================================================# 
def outline(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	edges = cv2.Canny(img,50,150, apertureSize = 3)
	cv2.imwrite(output_directory+"edges.jpg", edges)
	agreement = 100
	minLineLength = 30
	maxLineGap = 10

	lines = cv2.HoughLinesP(edges,1,np.pi/180, agreement,np.array([]),minLineLength,maxLineGap)
	drawLines(img, output_directory+"hough_lines.jpg", lines)

	return lines

def outline_with_sobel(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	edges = cv2.Canny(img,50,150, apertureSize = 3)

	laplacian = sobel_lines(img)
		
	agreement = 100
	minLineLength = 30
	maxLineGap = 10

	poop = cv2.imread("output/laplacian.jpg")
	pooper = cv2.cvtColor(poop, cv2.COLOR_BGR2GRAY)
	lines = cv2.HoughLinesP(pooper,1,np.pi/180, agreement,np.array([]),minLineLength,maxLineGap)
	drawLines(img, output_directory+"laplacian_lines.jpg", lines)

	return lines

def sobel_lines(filename, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	sobelx = cv2.Sobel(img,cv2.CV_64F,1,0,ksize=5) 
	# Calculation of Sobely 
	sobely = cv2.Sobel(img,cv2.CV_64F,0,1,ksize=5) 
	# Calculation of Laplacian 
	laplacian = cv2.Laplacian(img,cv2.CV_64F) 
	cv2.imwrite("output/laplacian.jpg",laplacian)
	return laplacian

#=================================================================================# 
#Filtering and Merging Hough Lines with Slope and Distance 
#=================================================================================#
## THIS IS WITH SLOPES. USE A METHOD WITH ANGLES TOO

def perpendicular_distance(x1, y1, lx1, ly1, lx2, ly2):
	slope, intercept = get_equation(lx1,ly1,lx2,ly2)
	if (slope == math.inf or intercept == math.inf):
		return abs(x1-lx1)
	else:
		a = 1
		b = -1*slope
		c = -1*intercept
		distance = abs((b * x1 + a * y1 + c)) / (math.sqrt(a * a + b * b))
		return distance 


def get_equation(x1,y1,x2,y2):
	if (x2-x1 == 0):
		return (math.inf, math.inf)
	slope = (y2-y1)/(x2-x1)
	intercept = y1-(slope*x1)
	return (slope, intercept)

def contains(lst, target, interval):
	if (target == math.inf and math.inf in lst):
		return math.inf
	for value in lst:
		if abs(target-value) <= interval:
			return value
	return None

def merge(lines1, lines2):
	lines = np.concatenate((lines1, lines2), axis = 0)
	return lines

def purify(lines, output_directory, filename):
	purified = {}
	pure = []
	img = cv2.imread(filename)

	for line in lines:
		(x1,y1),(x2,y2) = unpack(line)
		slope, intercept = get_equation(x1, y1, x2, y2)
		if (slope != math.inf):
			slope = int(round(slope))

		slope_exists = contains(purified, slope, 1)
		if slope_exists:
			slope = slope_exists
			existing_line = purified[slope][0][0]
			(lx1,ly1),(lx2,ly2)  = unpack(existing_line)
			distance = perpendicular_distance(x2, y2, lx1, ly1, lx2, ly2) 
			distance = int(10*round(distance/10))
			if (distance in purified[slope].keys()):
				purified[slope][distance].append(line)
			else:
				purified[slope][distance] = [line]
		else:
			purified[slope] = {}
			purified[slope][0] = [line]

	for slope, slope_clusters in purified.items():
		distances = slope_clusters.keys()
		smallest = 0
		biggest = 0
		for distance in distances:
			if distance < smallest:
				smallest = distance 
			if distance > biggest:
				 biggest = distance 
		left_endpoint1, left_endpoint2 = connect(slope_clusters[smallest])
		right_endpoint1, right_endpoint2 = connect(slope_clusters[biggest])
		pure.append([[left_endpoint1[0], left_endpoint1[1], left_endpoint2[0], left_endpoint2[1]]])
		pure.append([[right_endpoint1[0], right_endpoint1[1], right_endpoint2[0], right_endpoint2[1]]])
		cv2.line(img,left_endpoint1,left_endpoint2,(0,255,0),6)
		cv2.line(img,right_endpoint1,right_endpoint2,(0,255,0),6)
	cv2.imwrite(output_directory+"purified.jpg", img)
	return np.array(pure)

def connect(parallel_lines):
	endpoint1 = unpack(parallel_lines[0])[0]
	endpoint2 = unpack(parallel_lines[0])[1]
	for line in parallel_lines:
		(point1, point2) = unpack(line)
		minpoint = point1
		maxpoint = point2
		if (point1[0] > point2[0]):
			minpoint = point2
			maxpoint = point1
		if (minpoint[0]<endpoint1[0]):
			endpoint1 = minpoint
		if (maxpoint[0]>endpoint2[0]):
			endpoint2 = maxpoint
	return (endpoint1, endpoint2)

#=================================================================================# 
# Line Segment Detector Algorithm
#=================================================================================# 
# RUN LSD ON THINNED IMAGE SKELETON
def thin(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	size = np.size(img)
	skel = np.zeros(img.shape,np.uint8)
	 
	ret,img = cv2.threshold(img,127,255,0)
	element = cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3))
	done = False
	 
	while( not done):
	    eroded = cv2.erode(img,element)
	    temp = cv2.dilate(eroded,element)
	    temp = cv2.subtract(img,temp)
	    skel = cv2.bitwise_or(skel,temp)
	    img = eroded.copy()
	 
	    zeros = size - cv2.countNonZero(img)
	    if zeros==size:
	        done = True
	 
	cv2.imwrite(output_directory+"skeleton.jpg",skel)
	return skel

def LSD(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	detector = cv2.createLineSegmentDetector(0)
	lines = detector.detect(img)

	drawLines(img, output_directory+"lsd_lines.jpg", lines[0])

	return lines[0]
