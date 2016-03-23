#include "Slic.h"

Slic::Slic(Mat labImage, Mat sourceImage, const Settings& settings) : M(settings.M) {
	this->labImage = labImage;
	this->sourceImage = sourceImage;

	//Use Gaussian blur
	GaussianBlur(this->labImage, this->labImage, Size(settings.GaussianBlurX, settings.GaussianBlurY), 0);
	
	this->imageWidth = labImage.size().width;
	this->imageHeight = labImage.size().height;
	this->S = sqrt((imageWidth * imageHeight) / (double) settings.Superpixels);

	this->init();
	this->generate();
	if (!settings.SuperpixelatedFile.empty()){
		this->drawBack(settings.SuperpixelatedFile);
	}
}

Slic::~Slic() {
	
}
/*
Initialize variables for SLIC
*/
void Slic::init()
{
	this->centers.clear();
	this->centersColor.clear();
	this->pixelsInCluster.clear();

	this->distances.clear();
	this->clustersIndex.clear();

	//initialize longest possible length and clustersIndex to none for each pixel
	for( int x = 0; x < this->imageWidth; x++ )
	{
		vector<int> ci;
		vector<double> d;
		for(int y = 0; y < this->imageHeight; y++)
		{
			ci.push_back(-1);
			d.push_back(DBL_MAX);
		}
		this->clustersIndex.push_back(ci);
		this->distances.push_back(d);
	}
	
	//count centers of superpixels
	int maxWidth = this->imageWidth - this->S / 2;
	int maxHeight = this->imageHeight - this->S / 2;

	for( int x = this->S ; x < maxWidth; x += this->S )
	{
		for(int y = this->S; y < maxHeight; y += this->S)
		{
			//count localMinimum for center
			Point newCenter = this->localMinimum(Point(x, y));

			this->centers.push_back( newCenter );
			this->centersColor.push_back((Vec3f) this->labImage.at<Vec3f>(newCenter));

			this->pixelsInCluster.push_back(0);
		}
	}
}


/*
Generate superpixels
*/
void Slic::generate()
{
	// Instead of counting error, suggested is 10 loops
	for(int e1 = 0; e1 < 10; e1++)
	{
		// Clear counted distances from superpixel center
		if(e1 > 0)
		{
			for(int x = 0; x < this->imageWidth; x++)
			{
				for(int y = 0; y < this->imageHeight; y++)
				{
					this->distances[x][y] = DBL_MAX;
				}
			}
		}

		int centersCount = this->centers.size();
		for(int c = 0; c < centersCount; c++)
		{
			// Set "x" and "y" range for 2S x 2S area from center
			int initX = std::max((int)(this->centers[c].x - this->S), 0),
				initY = std::max((int)(this->centers[c].y - this->S), 0);

			int endX = std::min((int)(this->centers[c].x + this->S), this->imageWidth),
				endY = std::min((int)(this->centers[c].y + this->S), this->imageHeight);

			Point centerPoint = this->centers[c];

			Vec3f centerColor = this->centersColor[c];			

			// Iterate in 2S x 2S area around center
			for(int x = initX; x < endX; x++)
			{
				for(int y = initY; y < endY; y++)
				{
					Point pixelPoint = Point(x, y);
					Vec3f pixelColor = this->labImage.at<Vec3f>(pixelPoint);
					
					double distance = this->distance(centerPoint, centerColor, pixelPoint, pixelColor);
					// Set pixel to closest superpixel
					if(distance < this->distances[x][y])
					{
						this->distances[x][y] = distance;
						this->clustersIndex[x][y] = c;
					}
				}
			}
		}

		// Count new center of cluster as average color and coordinates
		// Clear variables
		for(int c = 0; c < centersCount; c++)
		{
			this->centers[c] = Point(0,0);
			this->centersColor[c] = Vec3f(0, 0, 0);
			this->pixelsInCluster[c] = 0;
		}
		// Sum all colors and coordinates
		for(int x = 0; x < this->imageWidth; x++)
		{
			for(int y = 0; y < this->imageHeight; y++)
			{
				int ci = clustersIndex[x][y];
				if(ci > -1)
				{
					this->centers[ci].x += x;
					this->centers[ci].y += y;
					this->centersColor[ci] += this->labImage.at<Vec3f>( y, x );
					this->pixelsInCluster[ci] += 1;
				}
			}
		}
		// Count average
		for(int c = 0; c < centersCount; c++)
		{
			this->centers[c].x /= this->pixelsInCluster[c];
			this->centers[c].y /= this->pixelsInCluster[c];

			this->centersColor[c] = this->centersColor[c] / this->pixelsInCluster[c];
		}
	}
}
/*
Draw superpixels back to image
*/
void Slic::drawBack(const string outputFile)
{
	Mat destImage = this->sourceImage.clone();
	vector<Vec3f> clustersColor;
	// Count average color of superpixels in BGR space
	for(int c = 0; c < this->centers.size(); c++)
	{
		clustersColor.push_back(Vec3f(0,0,0));
		this->pixelsInCluster[c] = 0;
	}
	// Sum colors
	for(int x = 0; x < this->imageWidth; x++)
	{
		for(int y = 0; y < this->imageHeight; y++)
		{
			int ci = clustersIndex[x][y];
			if(ci > -1)
			{

				clustersColor[ci] += destImage.at<Vec3b>(y, x);
				this->pixelsInCluster[ci] += 1;
			}
		}
	}
	// Count average
	for(int c = 0; c < this->centers.size(); c++)
	{
		clustersColor[c] = clustersColor[c] / this->pixelsInCluster[c];
	}
	// Put color back
	for(int x = 0; x < this->imageWidth; x++)
	{
		for(int y = 0; y < this->imageHeight; y++)
		{
			int ci = clustersIndex[x][y];
			if(ci > -1)
			{
				destImage.at<Vec3b>(y, x) = (Vec3b)clustersColor[ci];
			}
		}
	}

	cv::imwrite(outputFile, destImage);
}

/*
Count distance between two pixels
*/
double Slic::distance(const Point &point1,const Vec3f &color1, const Point &point2, const Vec3f &color2)
{
	double dc = sqrt(
		SQR(color1.val[0] - color2.val[0]) +
		SQR(color1.val[1] - color2.val[1]) +
		SQR(color1.val[2] - color2.val[2])
		);

	double ds = sqrt(
		SQR(point1.x - point2.x) +
		SQR(point1.y - point2.y)
		);

	return sqrt(SQR(dc / this->M) + SQR(ds / (int)this->S));
}

/*
Lowest gradient position at 3x3 neighborhood
*/
Point Slic::localMinimum(const Point &center)
{
	double localMin = DBL_MAX;
	Point localMinPoint = Point(center);

	//go through 3x3 neighborhood
	for(int x = center.x - 1; x < center.x + 2; x++)
	{
		for(int y = center.y - 1; y < center.y + 2; y++)
		{
			if(x < 0 || x > this->imageWidth || y < 0 || y > this->imageHeight)
			{
				continue;
			}



			Vec3f c1 = this->labImage.at<Vec3f>(y, x + 1);
			Vec3f c2 = this->labImage.at<Vec3f>(y, x - 1);

			Vec3f c3 = this->labImage.at<Vec3f>(y + 1, x);
			Vec3f c4 = this->labImage.at<Vec3f>(y - 1, x);

			//count min as || I(x + 1, y) - I(x - 1, y)||^2 + || I(x, y + 1) - I(x; y - 1) ||^2
			// ||..|| is L2 norm = sqrt( v1^2 + v2^2 + ... + vn^2)
			double l21 = sqrt(
				SQR(c1.val[0] - c2.val[0]) +
				SQR(c1.val[1] - c2.val[1]) +
				SQR(c1.val[2] - c2.val[2])
				);

			double l22 = sqrt(
				SQR(c3.val[0] - c4.val[0]) +
				SQR(c3.val[1] - c4.val[1]) +
				SQR(c3.val[2] - c4.val[2])
				);

			double min = SQR(l21) + SQR(l22);

			if(min < localMin)
			{
				localMin = min;
				localMinPoint.x = x;
				localMinPoint.y = y;
			}
		}
	}

	return localMinPoint;
}