#pragma once
#include "SuperPixel.h"

class SPPath
{
public:
	SPPath(SuperPixel*, SuperPixel*,double);
	~SPPath();

	SuperPixel* nextSuperPixel(SuperPixel*);
	SuperPixel* superPixelDeleting(SuperPixel*);
	
	inline double getLength() { return this->length; } //Get length of path

	bool isSame(SPPath*);

private:
	SuperPixel *sp1, *sp2; // End points of path
	double length; // Length of path
};

