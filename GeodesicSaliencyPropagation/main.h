#pragma once
#include <string>

#define SQR(x) ( (x) * (x) )

class GSPException : public std::exception
{
	public:
		GSPException(const char * const message) : std::exception(message){}
};

struct Settings{
	std::string FileName; //povinné
	std::string OutpuFileName; //povinné
	int Superpixels; //povinné
	double CornerDetectorThreshold; //povinné
	int M; // default 40
	int GaussianBlurX; //default 5
	int GaussianBlurY; //default 5
	std::string SuperpixelatedFile; //default prázdné
	bool IgnorePointsLimit; // default false
	std::string ConvexHullFile;

	double StepFuncThreshold; //default 0.005
	double StepFuncXOffset; // default 2.8
	double StepFuncYRange; //default 2
	double StepFuncSteepness; //default 4

	Settings() : M(40),
		GaussianBlurX(5),
		GaussianBlurY(5),
		IgnorePointsLimit(false),
		StepFuncThreshold(0.005),
		StepFuncXOffset(2.8),
		StepFuncYRange(2),
		StepFuncSteepness(4)
	{}
};