#pragma once
#include <maya/MPxNode.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MVector.h>

struct TransformComponents 
{
	MVector		translation;
	MQuaternion rotation;
	MVector	    scale;
};

class KSpaceSwitchMatrix : public MPxNode
{

private:
	// INPUT PORTS
	static MObject PARENT_INVERSE_MATRIX;

	static MObject TARGET;
	static MObject WEIGHT;
	static MObject TRANSLATE_ENABLE;
	static MObject ROTATE_ENABLE;
	static MObject SCALE_ENABLE;
	static MObject TARGET_MATRIX;
	static MObject OFFSET_MATRIX;

	// OUTPUT PORTS
	static MObject OUTPUT_MATRIX;

public:
	static MTypeId  TYPE_ID;
	static MString  TYPE_NAME;

	static constexpr double EPS{ 1.0e-8 };

public:
	KSpaceSwitchMatrix();
	virtual ~KSpaceSwitchMatrix() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;
	static  void*   creator();
	static  MStatus initialize();
	SchedulingType  schedulingType() const override { return SchedulingType::kParallel; }

	static void   setupUI();

	static TransformComponents decomposeMatrix(const MMatrix& matrix);
	static MMatrix			   blendMatrix    (const MMatrix& m1, const MMatrix& m2, double weight);

	template <typename T>
	static T blend(const T& v1, const T& v2, double weight)
	{
		// Returns a mixed value of v1 and v2 using the parameter weight
		return v1 + (v2 - v1) * weight;
	}


};