#pragma once
#include <maya/MPxNode.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>

class KChainRoll : public MPxNode
{

private:
	// input port
	static MObject COUNT;
	static MObject ROLL;
	static MObject ANGLE;
	static MObject FALLOFF;
	static MObject CURVE_DATA;

	// output port
	static MObject OUT_ANGLES;

public:
	static MTypeId  TYPE_ID;
	static MString  TYPE_NAME;

	static MFloatArray POSITIONS;
	static MFloatArray VALUES;
	static MIntArray   INTERPS;

public:
	KChainRoll();
	virtual ~KChainRoll() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;
	static  void*   creator();
	static  MStatus initialize();
	virtual void    postConstructor() override;

	static double clamp(double v, double _min=0.0, double _max=1.0);
	static double remap(double v, double inMin,    double inMax);
	static void   setupUI();

};