#include "GeodesicSaliencyPropagation.h"
#include <iomanip>


GeodesicSaliencyPropagation::GeodesicSaliencyPropagation(const Settings& settings) : settings(settings)
{
	this->sourceImage = imread(settings.FileName);
	if (!this->sourceImage.data){
		throw GSPException("Soubor nelze otevrit!");
	}
	this->maskImage = Mat::zeros(this->sourceImage.size(),CV_8UC1);

	this->imageWidth = this->sourceImage.size().width;
	this->imageHeight = this->sourceImage.size().height;

	//Convert to 32bit channel
	this->sourceImage.convertTo(this->labImage, CV_32FC3);
	this->labImage *= 1.0 / 255; //normalize
	cvtColor(this->labImage, this->labImage, CV_BGR2Lab); //Convert to CIELab

	this->doSlic();

	this->prepareSuperPixels();

	this->countPropagation();

	this->drawMask();

	imwrite(settings.OutpuFileName, this->maskImage);
	
}
/*
Create SLIC segmentation
*/
void GeodesicSaliencyPropagation::doSlic()
{
	unique_ptr<Slic> slic(new Slic(this->labImage, this->sourceImage, this->settings));

	this->clusterCenters = slic->getSuperPixelsCenters();
	this->clusterLABColors = slic->getSuperPixelsLABColors();
	this->slicStep = slic->getStep();
	this->clustersIndex = slic->getClustersIndexes();
	//this->maskImage = slic->returnBack();
}
/*
Count Global contrast, convexhull and coarse saliency map
Found neighbours for all superpixels and count length between them
*/
void GeodesicSaliencyPropagation::prepareSuperPixels()
{
	vector<Point> convexHull;
	//Find convex hull polygon
	this->getConvexHull(convexHull);

	unsigned centersCount = this->clusterCenters.size();

	double min = DBL_MAX, max = DBL_MIN;
	for (unsigned i = 0; i < centersCount; i++){
		double FiGlobal = 0;
		Vec3f *iColor = &this->clusterLABColors[i];
		int FiInformation = pointPolygonTest(convexHull, this->clusterCenters[i], false) >= 0 ? 1 : 0;

		// Count global contrast
		for (unsigned j = 0; j < centersCount; j++){
			Vec3f *jColor = &this->clusterLABColors[j];
			double norm = sqrt(
				SQR(iColor->val[0] - jColor->val[0]) +
				SQR(iColor->val[1] - jColor->val[1]) +
				SQR(iColor->val[2] - jColor->val[2])
				);
			FiGlobal += SQR(norm);
		}
		if (FiGlobal < min){
			min = FiGlobal;
		}
		if (FiGlobal > max){
			max = FiGlobal;
		}

		//Prepare SuperPixels objects
		this->superPixels.push_back(new SuperPixel(FiGlobal, FiInformation, i));

		unordered_set<int> clusterNeighbours;
		this->neighbours.push_back(clusterNeighbours);
		//find neighbours for each superpixel
		this->floodFill(i, this->clusterCenters[i].x, this->clusterCenters[i].y);

		//circle(this->maskImage, this->clusterCenters[i], 2, /*FiInformation == 1 ?*/ Scalar(0, 255, 0) /*: Scalar(255, 0, 0)*/, 2);
		//imwrite("asdf.png", this->maskImage);
	}

	// Normalize global contrast
	double divisor = max - min;
	for (unsigned i = 0; i < centersCount; i++){
		this->superPixels[i]->setFiGlobal((this->superPixels[i]->getFiGlobal() - min) / divisor);
	}

	// Count length for superpixels which have path to each other
	for (unsigned i = 0; i < clusterCenters.size(); i++){
		for (const int l : this->neighbours[i]){
			//line(this->maskImage, this->clusterCenters[i], this->clusterCenters[l], Scalar(0, 255, 0), 1);

			Vec3f color1 = this->clusterLABColors[i],
				  color2 = this->clusterLABColors[l];

			double length = sqrt(
				SQR(color1.val[0] - color2.val[0]) +
				SQR(color1.val[1] - color2.val[1]) +
				SQR(color1.val[2] - color2.val[2])
				);
			this->superPixels[i]->addPath(this->superPixels[l], length);
		}
		//putText(this->superPixelatedImage, to_string(i), this->clusterCenters[i], FONT_HERSHEY_PLAIN, 0.8, Scalar(255, 255, 255));
	}
	//imwrite("floodfill.png", this->maskImage);
}

/*
Find neighbours by flood fill algorithm
*/
void GeodesicSaliencyPropagation::floodFill(int startIndex, int x, int y)
{
	stack<Point> points;
	points.push(Point(x, y));

	while (!points.empty())
	{
		x = points.top().x;
		y = points.top().y;
		points.pop();

		if (x < 0 || y < 0 || x >= this->imageWidth || y >= this->imageHeight){
			continue;
		}

		int negative = FLOOD_FILL_NEGATIVE(this->clustersIndex[x][y]);
		if (negative == startIndex){ // Negative index of negative index -> positive index is same as start, I visited this pixel
			continue;
		}

		if (this->clustersIndex[x][y] == startIndex){ // Iterate over all pixels which belongs to same superpixel
			this->clustersIndex[x][y] = negative;

			points.push(Point(x + 1, y)); //down
			points.push(Point(x, y + 1)); //right
			points.push(Point(x - 1, y)); //up
			points.push(Point(x, y - 1)); //left
		}
		else
		{	
			// I found different index -> different superpixel
			int insert = this->clustersIndex[x][y];
			if (insert < 0){ // If index is negative, it was visited earlier
				insert = FLOOD_FILL_NEGATIVE(insert);
			}
			this->neighbours[startIndex].insert(insert); // New index, new neighbout
		}
	}

	// Old implementation with recursive
	/*if (x < 0 || y < 0 || x >= this->imageWidth || y >= this->imageHeight){
		return;
	}

	int negative = FLOOD_FILL_NEGATIVE(this->clustersIndex[x][y]);

	if (negative == startIndex){
		return;
	}

	if (this->clustersIndex[x][y] == startIndex){
		this->clustersIndex[x][y] = negative;

		this->floodFill(startIndex, x + 1, y); //up
		this->floodFill(startIndex, x, y + 1); //right
		this->floodFill(startIndex, x - 1, y); //down
		this->floodFill(startIndex, x, y - 1); //left
	}else{
		int insert = this->clustersIndex[x][y];
		if (insert < 0){
			insert = FLOOD_FILL_NEGATIVE(insert);
		}
		this->neighbours[startIndex].insert(insert);
	}*/
}

/*
Count propagation between every two superpixels.
*/
void GeodesicSaliencyPropagation::countPropagation()
{
	unsigned superPixelsCount = this->superPixels.size();

	for (unsigned i = 0; i < superPixelsCount; i++){
		SuperPixel* superPixel = this->superPixels[i];
		
		double N = 0.0, SiPropagation = 0.0;
		//Get shortest paths from current superpixels to all others
		vector<double> lengthOfPaths = this->shortestPaths(superPixel);

		// count across all superpixels
		for (unsigned j = 0; j < superPixelsCount; j++){
			N += exp(-0.05 * lengthOfPaths[j]);
		}

		for (unsigned j = 0; j < superPixelsCount; j++){
			double SjCoarse = superPixels[j]->getSiCoarse();

			SiPropagation += (1 / N) * exp(-0.05 * lengthOfPaths[j]) * SjCoarse;
		}
		superPixel->setSiPropagation(SiPropagation);
	}
}

/*
Draw clipping mask
*/
void GeodesicSaliencyPropagation::drawMask()
{
	unsigned superPixelsCount = this->superPixels.size();
	vector<double> propagations(superPixelsCount);
	double min = DBL_MAX, max = DBL_MIN;

	//Get range of propagation
	for (unsigned i = 0; i < superPixelsCount; i++){
		double propagation = this->superPixels[i]->getSiPropagation();
		if (propagation < min){
			min = propagation;
		}
		if (propagation > max){
			max = propagation;
		}
		propagations[i] = propagation;
	}

	//Normalize propagation
	double divisor = max - min;
	for (unsigned i = 0; i < superPixelsCount; i++){
		propagations[i] = round(((propagations[i] - min) / divisor) * 255);
	}

	//Draw back normalized values
	for (int x = 0; x < this->imageWidth; x++){
		for (int y = 0; y < this->imageHeight; y++){
			int clusterIndex = this->clustersIndex[x][y];
			if (clusterIndex < 0){
				clusterIndex = FLOOD_FILL_NEGATIVE(clusterIndex);
			}
			this->maskImage.at<uchar>(y, x) = (uchar)propagations[clusterIndex];
		}
	}
}

/*
Get shortest paths from sourceSuperPixel to all others
*/
vector<double> GeodesicSaliencyPropagation::shortestPaths(SuperPixel* sourceSuperPixel)
{
	unsigned superPixelsCount = this->superPixels.size();
	vector<double> lengthOfPaths(superPixelsCount);

	//set to all superpixels "infinite" length and non processed flag
	for (unsigned i = 0; i < superPixelsCount; i++){
		this->superPixels[i]->setProcessedFlag(false);
		lengthOfPaths[i] = DBL_MAX;
	}
	//set length 0 to source superpixel 
	lengthOfPaths[sourceSuperPixel->getGraphIndex()] = 0.0;
	for (unsigned i = 0; i < superPixelsCount - 1; i++){
		
		//select min length of path on non processed superpixels
		double minLength = DBL_MAX;
		int minIndex;
		for (unsigned m = 0; m < superPixelsCount; m++){
			if (this->superPixels[m]->isProcessed() == false && lengthOfPaths[m] < minLength){
				minLength = lengthOfPaths[m];
				minIndex = m;
			}
		}

		//Select closest superpixel, set as processed and select all paths to others superpixels
		SuperPixel *current = this->superPixels[minIndex];
		current->setProcessedFlag(true);
		vector<SPPath*> paths = current->getPaths();

		//Travel through all paths to non processed superpixels
		for (unsigned p = 0; p < paths.size(); p++){
			SuperPixel *dest = paths[p]->nextSuperPixel(current);
			if (dest->isProcessed() == false){
				int destIndex = dest->getGraphIndex();
				double deltaPk = paths[p]->getLength();

				double lengthToDest = lengthOfPaths[minIndex] + STEP_FUNCTION(deltaPk - this->settings.StepFuncThreshold, this->settings.StepFuncYRange, this->settings.StepFuncXOffset, this->settings.StepFuncSteepness) * deltaPk;
				//save closest path
				if (lengthToDest < lengthOfPaths[destIndex]){
					lengthOfPaths[destIndex] = lengthToDest;
				}
			}
		}
	}
	return lengthOfPaths;
}

/*
Detect corners and get convex hull of them
*/
void GeodesicSaliencyPropagation::getConvexHull(vector<Point>& convexHull)
{
	vector<Point> corners;
	Mat imageGray, dst;
	cvtColor(this->sourceImage, imageGray, CV_BGR2GRAY);

	// Set disabled area on edges
	int boundary = (int)(this->slicStep * 1.5);
	int boundaryWidth = this->labImage.size().width - boundary,
		boundaryHeight = this->labImage.size().height - boundary,
		pointsLimit = (int)((boundaryWidth - boundary) * (boundaryHeight - boundary) * 0.4);

	if (this->settings.IgnorePointsLimit)
	{
		pointsLimit = INT_MAX;
	}

	// Use Harris corner detector
	dst = Mat::zeros(imageGray.size(), CV_32FC1);
	cornerHarris(imageGray, dst, 2, 3, 0.04);
	normalize(dst, dst, 0, 255, NORM_MINMAX, CV_32FC1, Mat());

	//imwrite("harriscorner.png", dst);

	// Save corners
	for (int y = boundary; y < boundaryHeight; y++)
	{
		
		if ((int)corners.size() > pointsLimit)
		{
			throw GSPException("Bodu pro konvexni obal je vice nez 40% z celkoveho poctu pixelu.\nNastavte vyssi hodnotu parametru CT nebo ignorujte vysoke pocty bodu prepinace -IPL");
		}
		for (int x = boundary; x < boundaryWidth; x++)
		{
			// Save only higher than threshold
			if (dst.at<float>(y, x) > this->settings.CornerDetectorThreshold)
			{
				corners.push_back(Point(x, y));
			}
		}
	}
	if (corners.size() < 3)
	{
		throw GSPException("Bodu pro konvexni obal je mene nez 3.\nSnizte hodnotu parametru CT");
	}

	// Get convex hull
	cv::convexHull(Mat(corners), convexHull);
	// Draw convex hull if is wanted
	if (!this->settings.ConvexHullFile.empty()){
		Mat convexHullImage = this->sourceImage.clone();
		for (unsigned i = 0; i < corners.size(); i++){
			circle(convexHullImage, corners[i], 4, Scalar(30, 220, 220), 1);
		}

		for (unsigned i = 0; i < convexHull.size(); i++){
			int j = (i + 1) % convexHull.size();
			cv::line(convexHullImage, convexHull[i], convexHull[j], Scalar(50, 50, 230), 2);
		}
		imwrite(this->settings.ConvexHullFile, convexHullImage);
	}
}


GeodesicSaliencyPropagation::~GeodesicSaliencyPropagation()
{
	for (unsigned int i = 0; i < this->superPixels.size(); i++){
		delete this->superPixels[i];
	}
}
