import cv2
import numpy as np
import math
from matplotlib import pyplot as plt

def preprocess(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
	cv2.imwrite(output_directory+"GRAYSCALE.jpg",gray)
	return gray

def filter(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	blur = cv2.GaussianBlur(img,(3,3),0)
	cv2.imwrite(output_directory+"GaussianBlur.jpg",blur)
	return blur

def gradient_elimination(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	ret,thresh = cv2.threshold(img,200,255,cv2.THRESH_OTSU)
	cv2.imwrite(output_directory+"THRESH_OTSU.jpg",thresh)
	return thresh

def outline(filename, output_directory, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	edges = cv2.Canny(img,50,150, apertureSize = 3)

	laplacian = sobel_lines(img)

	agreement = 100
	minLineLength = 30
	maxLineGap = 10

	img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
	lines = cv2.HoughLinesP(edges,1,np.pi/180, agreement,np.array([]),minLineLength,maxLineGap)
	for line in lines:
		for x1,y1,x2,y2 in line:
			cv2.line(img,(x1,y1),(x2,y2),(0,255,0),6)

	cv2.imwrite(output_directory+"laplacian_lines.jpg",img)

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

	img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
	poop = cv2.imread("output/laplacian.jpg")
	pooper = cv2.cvtColor(poop, cv2.COLOR_BGR2GRAY)
	lines = cv2.HoughLinesP(pooper,1,np.pi/180, agreement,np.array([]),minLineLength,maxLineGap)
	for line in lines:
		for x1,y1,x2,y2 in line:
			cv2.line(img,(x1,y1),(x2,y2),(0,255,0),6)

	cv2.imwrite(output_directory+"laplacian_lines.jpg",img)

	return lines

def sobel_lines(filename, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	# while (1):
	# 	# Calcution of Sobelx 
	# 	sobelx = cv2.Sobel(img,cv2.CV_64F,1,0,ksize=5) 
	# 	# Calculation of Sobely 
	# 	sobely = cv2.Sobel(img,cv2.CV_64F,0,1,ksize=5) 
	# 	# Calculation of Laplacian 
	# 	laplacian = cv2.Laplacian(img,cv2.CV_64F) 
	# 	cv2.imshow('sobelx',sobelx) 
	# 	cv2.imshow('sobely',sobely) 
	# 	cv2.imshow('laplacian',laplacian) 

	# 	k = cv2.waitKey(5) & 0xFF
	# 	if k == 27: 
	# 		break
	sobelx = cv2.Sobel(img,cv2.CV_64F,1,0,ksize=5) 
	# Calculation of Sobely 
	sobely = cv2.Sobel(img,cv2.CV_64F,0,1,ksize=5) 
	# Calculation of Laplacian 
	laplacian = cv2.Laplacian(img,cv2.CV_64F) 
	cv2.imwrite("output/laplacian.jpg",laplacian)
	return laplacian

def drawLines(filename, output, lines):
	img = cv2.imread(filename)
	for line in lines:
		for x1,y1,x2,y2 in line:
			cv2.line(img,(x1,y1),(x2,y2),(0,255,0),6)
	cv2.imwrite(output,img)

#=================================================================================

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

def unpack(line):
	for x1,y1,x2,y2 in line:
		return ((x1,y1),(x2,y2))

# compare the outline from thresholding versus the outline from just blurring 
# start with the one from blurring, and add any distinct lines unique to thresholding
# currently this is N^2 time complexity. Optimize
def merge(lines1, lines2):
	lines = np.concatenate((lines1, lines2), axis = 0)
	return lines
# group together lines by slope and connect these groups into long line segments

# compare the outline from thresholding versus the outline from just blurring 
# start with the one from blurring, and add any distinct lines unique to thresholding
# currently this is N^2 time complexity. Optimize
#
def purify(lines, filename):
	purified = {}
	img = cv2.imread(filename)

	for line in lines:
		for x1,y1,x2,y2 in line:
			slope, intercept = get_equation(x1, y1, x2, y2)
			if (slope != math.inf):
				slope = int(round(slope))

			slope_exists = contains(purified, slope, 1)
			if slope_exists:
				slope = slope_exists
				existing_line = purified[slope][0][0]
				for lx1,ly1,lx2,ly2 in existing_line:
					distance = perpendicular_distance(x2, y2, lx1, ly1, lx2, ly2) 
					distance = int(10*round(distance/10))
				if (distance in purified[slope].keys()):
					purified[slope][distance].append(line)
				else:
					purified[slope][distance] = [line]
			else:
				purified[slope] = {}
				purified[slope][0] = [line]

	# for slope, slope_clusters in purified.items():
	# 	for distance, distance_clusters in slope_clusters.items():
	# 		drawLines("test/test.png", "output/purified"+str(slope)+str(distance)+".jpg", distance_clusters)
	# 		endpoint1, endpoint2 = connect(distance_clusters)
	# 		cv2.line(img,endpoint1,endpoint2,(0,255,0),6)
	# 		cv2.imwrite("output/test.jpg", img)

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
		cv2.line(img,left_endpoint1,left_endpoint2,(0,255,0),6)
		cv2.line(img,right_endpoint1,right_endpoint2,(0,255,0),6)
		cv2.imwrite("output/test.jpg", img)

#Thinnning algorithm dilates lines then thins to one pixel. This merges all lines
#Then apply hough transform to get the endpoints (or corner detection to get endooints) 
#For this, 1 pixel might be too thin so then dilates again so its thick enough
def thin():
	return

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



# if two lines have the same slope and y interdept, group them
# if two lines have the same slope but different y intercepts, they are parallel
# for parellel lines, check perpendicular distance 
#=================================================================================

def main():
	INPUT = 'test/test.png'
	OUTPUT = 'output/'
	
	gray = preprocess(INPUT, OUTPUT)
	blur = filter(gray, OUTPUT, False)
	threshold = gradient_elimination(blur, OUTPUT, False)
	# threshold = gradient_elimination(INPUT, OUTPUT)
	lines1 = outline(blur, OUTPUT+"blurred_", False)
	lines2 = outline(threshold, OUTPUT+"threshold_", False)
	lines = merge(lines1, lines2)
	purify(lines, INPUT)

if __name__ == '__main__':	
	main()




