#pragma once
#include <vector>

class SPPath;

using namespace std;
class SuperPixel
{
private:
	vector<SPPath*> paths; // All paths from and to this superpixel
	double FiGlobal; //Global contrast of superpixel
	int FiInformation; // Information about position in/out of convexhull
	double SiPropagation; // Propagation of superpixel

	bool savePath(SPPath*);

	int graphIndex; // Unique index of superpixel
	bool processed; // Processed flag to Dijkstra algorithm

public:
	SuperPixel(double, int, int);
	~SuperPixel();
	
	void addPath(SuperPixel*, double);
	inline double getFiGlobal() { return this->FiGlobal; }
	inline void setFiGlobal(double FiGlobal) { this->FiGlobal = FiGlobal; }

	inline double getSiPropagation() { return this->SiPropagation; }
	inline void setSiPropagation(double SiPropagation) { this->SiPropagation = SiPropagation; }

	inline int getFiInformation() { return this->FiInformation; }
	inline void setFiGlobal(int FiInformation) { this->FiInformation = FiInformation; }

	inline int getGraphIndex() { return this->graphIndex; }

	inline double getSiCoarse() { return this->FiGlobal * this->FiInformation; }

	inline vector<SPPath*> getPaths() { return this->paths; }

	inline bool isProcessed() { return this->processed; }
	inline void setProcessedFlag(bool processed) { this->processed = processed; }
};