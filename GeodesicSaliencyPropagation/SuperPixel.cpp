#include "SuperPixel.h"
#include "SPPath.h"

SuperPixel::SuperPixel(double FiGlobal, int FiInformation, int graphIndex)
{
	this->FiGlobal = FiGlobal;
	this->FiInformation = FiInformation;
	this->graphIndex = graphIndex;
	this->processed = false;
}

/*
Add path from this superpixel to another
*/
void SuperPixel::addPath(SuperPixel *to, double length){
	SPPath *path = new SPPath(this, to, length);

	bool saved1 = this->savePath(path);
	bool saved2 = to->savePath(path);

	if (!saved1 && !saved2){
		delete path;
		return;
	}
}
/*
Save path if doesn't exists
*/
bool SuperPixel::savePath(SPPath *path){
	for (unsigned i = 0; i < paths.size(); i++){
		SPPath* iPath = paths[i];
		if (path->isSame(iPath)){
			return false;
		}
	}
	paths.push_back(path);
	return true;
}

SuperPixel::~SuperPixel()
{
	for (unsigned int i = 0; i < this->paths.size(); i++){
		SPPath* path = this->paths[i];
		SuperPixel* next = path->superPixelDeleting(this);
		if (next == nullptr){
			delete path;
		}
	}
}