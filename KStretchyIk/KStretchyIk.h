#pragma once
#include <maya/MPxNode.h>
#include <maya/MVector.h>
#include <maya/MString.h>  
#include <maya/MTypeId.h> 
#include <maya/MObject.h>


class KStretchyIk : public MPxNode
{

private:
	// INPUT PORTS
	static MObject REST_UPR_LENGTH;
	static MObject REST_LWR_LENGTH;

	static MObject MULT_UPR_LENGTH;
	static MObject MULT_LWR_LENGTH;
	static MObject STRETCH;
	static MObject SOFT;
	static MObject SLIDE;
	static MObject PIN;

	static MObject ROOT_MATRIX;
	static MObject POLE_MATRIX;
	static MObject GOAL_MATRIX;
	static MObject INVERT;

	// OUTPUT PORTS
	static MObject OUT_UPR_LENGTH;
	static MObject OUT_LWR_LENGTH;
	static MObject OUT_IK_HANDLE_LOCAL_MATRIX;

public:
	static MTypeId TYPE_ID;
	static MString TYPE_NAME;
	static double  MAIN_EPS;

public:
	KStretchyIk		    ();
	virtual ~KStretchyIk() override;

	virtual MStatus compute   (const MPlug& plug, MDataBlock& dataBlock) override;
	static  void*   creator   ();
	static  MStatus initialize();
	static  void    setupUI   ();

	template <typename T>
	static T mix(const T& v1, const T& v2, double weight)
	{
		/*
		Returns a mixed value of v1 and v2 using the parameter weight
		*/
		return v1 + (v2 - v1) * weight;
	}

};