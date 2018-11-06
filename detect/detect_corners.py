import cv2
import numpy as np
import matplotlib.pyplot as plt

# Harris Corner 
def harris_corners(filename, isPath = False, blur = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
	if (blur):
		blur = cv2.GaussianBlur(gray,(3,3),0)
		blur = np.float32(blur)
	else:
		blur = gray
	dst = cv2.cornerHarris(blur,2,3,0.04)
	dst = cv2.dilate(dst,None, iterations = 3)

	img[dst>0.01*dst.max()] = [0,255,0]

	plt.imshow(img,cmap = 'gray')
	plt.title('Corners'), plt.xticks([]), plt.yticks([])

	plt.show()

# Max Harris Corner
def max_harris_corners(filename, isPath = False, blur = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
	if (blur):
		blur = cv2.GaussianBlur(gray,(3,3),0)
		blur = np.float32(blur)
	else:
		blur = gray
	# find Harris corners
	dst = cv2.cornerHarris(blur,2,3,0.04)
	dst = cv2.dilate(dst,None)
	ret, dst = cv2.threshold(dst,0.01*dst.max(),255,0)
	dst = np.uint8(dst)

	# find centroids
	ret, labels, stats, centroids = cv2.connectedComponentsWithStats(dst)

	# define the criteria to stop and refine the corners
	criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 0.001)
	corners = cv2.cornerSubPix(gray,np.float32(centroids),(5,5),(-1,-1),criteria)
	
	# # Now draw them
	# res = np.hstack((centroids,corners))
	# res = np.int0(res)
	# img[res[:,1],res[:,0]]=[0,0,255]
	# img[res[:,3],res[:,2]] = [0,255,0]

	# cv2.imwrite('subpixel5.png',img)

	return corners

# Shi Tomasi

def shitomasi(filename, isPath = False):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
	corners = cv2.goodFeaturesToTrack(gray, 10, 0.05, 25)
	corners = np.float32(corners)
	# for item in corners:
	#     x, y = item[0]
	#     cv2.circle(img, (x,y), 5, (0,255,0), -1)

	# cv2.imshow("Top corners", img)
	# cv2.waitKey()

	return np.squeeze(corners, axis = 1)

def merge():
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


# Thinning Algorithm returns image that can be fed into harris or max harris or shi tomasi




merge()


