# Created by Richard Bao and Shahrouz Ryan Alimo on 10/20/18.
# Copyright © Shahrouz Ryan Alimo, California Institute of Technology. All rights reserved.

import cv2
import numpy as np
import math
from matplotlib import pyplot as plt
from reduce_image import*
from detect_lines import*
from detect_corners import*

# Future: implement K MEANS TO FIND CLUSTERS WHERE ALL THREE PIPELINES AGREE

def significant_points(filename, corners_list, output_directory, isPath = True):
	colors = [(255,0,0),(0,255,0),(0,0,255)]
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
	index = 0
	for corners in corners_list:
		for point in corners:
			x, y = point
			cv2.circle(img, (x,y), 2, colors[index], -1)
		index += 1
		index = index%3
	cv2.imwrite(output_directory,img)


def main():
	INPUT = 'test/test.png'
	OUTPUT = 'output/'

	gray = preprocess(INPUT, OUTPUT)
	blur = filter(gray, OUTPUT, isPath=False)
	threshold = gradient_elimination(blur, OUTPUT, isPath=False)

	#pipeline 1	(hough)
	hough_lines1 = outline(blur, OUTPUT+"blurred_", False)
	hough_lines2 = outline(threshold, OUTPUT+"threshold_", False)
	hough_lines = merge(hough_lines1, hough_lines2)
	purified_hough = purify(hough_lines, OUTPUT+"hough_", INPUT)

	#pipelines 2 (LSD)
	skel = thin(threshold, OUTPUT, False)
	LSD_lines = LSD(skel, OUTPUT+"skeleton_", False)
	purified_LSD = purify(LSD_lines, OUTPUT+"lsd_", INPUT)


	#pipeline 3 (corner)
	corners = shitomasi(blur, OUTPUT, number = 15, isPath=False)


	points1 = []
	points2 = []
	for line in purified_hough:
		p1, p2 = unpack(line)
		points1.append(p1)
		points1.append(p2)
	for line in purified_LSD:
		p1, p2 = unpack(line)
		points2.append(p1)
		points2.append(p2)
	significant_points(threshold, [corners, points1, points2], OUTPUT+"significant_points.jpg", isPath = False)


	#make function to extract endpoints from hough_lines and LSD_lines
	#feed those into drawCorners


if __name__ == '__main__':	
	main()




