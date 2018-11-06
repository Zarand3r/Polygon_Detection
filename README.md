Robust polygon detection in satellites for use in computer vision babsed pose estimation.

**DETECTOR**
Image processing
================
Convert images to grayscale
Apply 3X3 Gaussian Filter for gradient elimination
Apply OTSU Thresholding on filtered image

Pipeline 1
==========
Detect lines with HOUGH Transform algorithm on blurred image
Detect lines with HOUGH Transform algorithm on threshold image
Combine results
Purify lines by merging similar, continuous lines and eliminating double edges
Extract interesting points as endpoints

Pipeline 2
==========
Use pixel thinning algorithm to generate skeletonized image
Detect lines on skeletonized image with LSD
Purify lines by merging similar, continuous lines and eliminating double edges
Extract interesting points as endpoints

Pipeline 3
==========
Apply Harris Corner Detector
Apply Shit Tomasi "Good Features" corner detector
Extract interesting points as corners

Deciding Significant Points
===========================
Compare the results of the three pipelines.
A cluster where all three pipelines agree is likely to be a significant point
Use K-means distance to decide this probabilistically (using a quad tree structure)
	- THIS STILL NEEDS TO BE IMPLEMENTED

**TRACKER**
Developed by Dr. Shahrouz Ryan Alimo, lead machine learning scientist at NASA JPL
