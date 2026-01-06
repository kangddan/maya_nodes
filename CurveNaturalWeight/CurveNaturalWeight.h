#pragma once
#include <maya/MPxNode.h>

class CurveNaturalWeight : public MPxNode
{
private:
		static MObject INPUT_CURVE; 
		static MObject WEIGHT; 

		static MObject OUTPUT_WEIGHT;

public:
	static MTypeId TYPE_ID;
	static MString TYPE_NAME;

public:
	CurveNaturalWeight();
	virtual ~CurveNaturalWeight() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;
	static void* creator();
	static MStatus initialize();

	static void setupUI();
};
