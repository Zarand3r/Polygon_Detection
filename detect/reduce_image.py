# Created by Richard Bao on 10/20/18.
# Copyright © 2018 Richard Bao, California Institute of Technology. All rights reserved.

import cv2
import numpy as np

#=================================================================================# 
# Image Reduction 
#=================================================================================# 
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