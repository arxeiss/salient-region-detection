#include "SPPath.h"


SPPath::SPPath(SuperPixel *sp1, SuperPixel *sp2, double length)
{
	this->sp1 = sp1;
	this->sp2 = sp2;
	this->length = length;
}


SPPath::~SPPath()
{
}

/*
Check if path is same as this, check end points
*/
bool SPPath::isSame(SPPath* path){
	if (path == nullptr){
		return false;
	}
	if (path->sp1 == this->sp1 && path->sp2 == this->sp2 ||
		path->sp1 == this->sp2 && path->sp2 == this->sp1){
		return true;
	}
	return false;
}
/*
Get second end point of path
*/
SuperPixel* SPPath::nextSuperPixel(SuperPixel* from){
	if (from == sp1){
		return sp2;
	}
	if (from == sp2){
		return sp1;
	}
	return nullptr;
}
/*
After deleting SuperPixel, need to clear paths too
*/
SuperPixel* SPPath::superPixelDeleting(SuperPixel* deleting){
	if (this->sp1 == deleting){
		this->sp1 = nullptr;
		return this->sp2;
	}
	if (this->sp2 == deleting){
		this->sp2 = nullptr;
		return this->sp1;
	}
	return nullptr;
}