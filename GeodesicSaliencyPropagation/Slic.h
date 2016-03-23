#pragma once

#include <iostream>
#include "opencv2/core/core.hpp"
#include <opencv2\imgproc\imgproc.hpp> //cvtColor
#include "opencv2/highgui/highgui.hpp" //imread, imwrite, waitkey

#include <math.h>
#include <vector>
#include <float.h>
#include "main.h"
#include <string>

using namespace cv;
using namespace std;

class Slic {
    private:
		const int M; //constant in figure 1
		double S; // step
		int imageWidth, imageHeight;
		Mat labImage, sourceImage;
		
		vector<Point> centers;// Coordinates of centers, changing every loop in generate
		vector<Vec3f> centersColor;// LAB color of centers in procesing image, changing every loop in generate

		vector<int> pixelsInCluster;// number of pixels in cluster

		vector<vector<double>> distances; // smaller distance for each pixel
		vector<vector<int>> clustersIndex; // index of cluster where pixel belongs

		void init();
		void generate();
		void drawBack(const string);
		void create_connectivity();

		double distance(const Point&, const Vec3f&, const Point&, const Vec3f& );

		Point localMinimum(const Point&);


    public:
        /* Constructor and destructor. */
		Slic(Mat, Mat, const Settings&);
        ~Slic();
		
		//Mat returnBack();

		inline vector<Point> getSuperPixelsCenters(){ return this->centers; }
		inline vector<Vec3f> getSuperPixelsLABColors(){ return this->centersColor; }
		inline vector<vector<int>> getClustersIndexes(){ return this->clustersIndex; }
		inline double getStep(){ return this->S; }
};