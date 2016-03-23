#pragma once

#include <iostream>
#include <math.h>
#include <vector>
#include <unordered_set>
#include <stack>
#include <memory> //unique_ptr...

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp" //cvtColor
#include "opencv2/highgui/highgui.hpp" //imread, imwrite, waitkey

#include "main.h"
#include "slic.h"
#include "SuperPixel.h"
#include "SPPath.h"

/*
Function to convert index of superpixel to negative numbers and back
*/
#define FLOOD_FILL_NEGATIVE(x) (((x) * (-1)) - 1)

/*
Step function in counting length of path in Dijkstra algorithm
*/
#define STEP_FUNCTION(x, yRange, xOffset, steepness) ( (yRange) / (1 + exp( ((x) - (xOffset)) * -1 * (steepness))) )

using namespace std;
using namespace cv;

class GeodesicSaliencyPropagation
{
private:
	Mat sourceImage, labImage, maskImage; 
	vector<Point> clusterCenters; // Coordinates of centers in each cluster (superpixel)
	vector<Vec3f> clusterLABColors; // Color of every cluster (superpixel)
	vector<SuperPixel*> superPixels; // Vector of SuperPixel object to use in Dijkstra algorithm
	vector<vector<int>> clustersIndex; // Index of cluster where belong every pixel in [x][y]
	vector<unordered_set<int>> neighbours; // Set of neighbours of every superpixel
	const Settings settings; // Settings from parameters

	int imageWidth, imageHeight;
	double slicStep;

	void floodFill(int, int, int);
	void doSlic();
	void prepareSuperPixels();
	void getConvexHull(vector<Point>&);
	void countPropagation();
	vector<double> shortestPaths(SuperPixel*);
	void drawMask();
public:
	GeodesicSaliencyPropagation(const Settings&);
	~GeodesicSaliencyPropagation();
};

