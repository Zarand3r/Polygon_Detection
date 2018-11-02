import cv2
import numpy as np
from matplotlib import pyplot as plt




# filename = 'test/test4.png'
# img = cv2.imread(filename)
# gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
# blur = cv2.GaussianBlur(gray,(5,5),0)
# ret,thresh = cv2.threshold(blur,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)  

# #use thresh

# edges = cv2.Canny(thresh,50,150,apertureSize = 3)

# agreement = 50
# minLineLength = 50
# maxLineGap = 10
# lines = cv2.HoughLinesP(edges,1,np.pi/180,agreement,minLineLength,maxLineGap)
# for line in lines:
# 	for x1,y1,x2,y2 in line:
# 	    cv2.line(img,(x1,y1),(x2,y2),(0,255,0),2)

# cv2.imwrite('../houghlines5.jpg',img)


def preprocess(filename, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
	return gray

def filter(filename, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	blur = cv2.GaussianBlur(img,(3,3),0)
	return blur

def gradient_elimination(filename, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	ret,thresh = cv2.threshold(img,200,255,cv2.THRESH_OTSU)
	return thresh

def outline(filename, isPath = True):
	img = filename
	if (isPath):
		img = cv2.imread(filename)
	edges = cv2.Canny(img,50,150, apertureSize = 3)

	lines = cv2.HoughLines(edges,1,np.pi/180,100, 30, 10)
	img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
	for line in lines:
		rho,theta = line[0]
		a = np.cos(theta)
		b = np.sin(theta)
		x0 = a*rho
		y0 = b*rho
		x1 = int(x0 + 1000*(-b))
		y1 = int(y0 + 1000*(a))
		x2 = int(x0 - 1000*(-b))
		y2 = int(y0 - 1000*(a))
		cv2.line(img,(x1,y1),(x2,y2),(0,255,0),6)

	plt.subplot(121),plt.imshow(img,cmap = 'gray')
	plt.title('Original Image'), plt.xticks([]), plt.yticks([])
	plt.subplot(122),plt.imshow(edges,cmap = 'gray')
	plt.title('Edge Image'), plt.xticks([]), plt.yticks([])

	plt.show()

	cv2.imwrite('houghlines3.jpg',img)


def main():
	INPUT = 'test/test2.png'
	
	gray = preprocess(INPUT)
	blur = filter(gray, False)
	threshold = gradient_elimination(blur, False)
	lines = outline(threshold, False)

if __name__ == '__main__':	
	main()



