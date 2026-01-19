#pragma once
#include <maya/MPxNode.h>
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MPoint.h>
#include <maya/MFnNurbsCurve.h>

#include <vector>

struct Segment
{
	MMatrix startMatrix;
	MMatrix endMatrix;

	std::vector<MVector> tangents;
	std::vector<MPoint>  points;
	std::vector<double>  blendWeights;

	Segment(const MMatrix& start, const MMatrix& end)
		: startMatrix(start), endMatrix(end)
	{
	}
};


class KNoFlipSplineSolver : public MPxNode
{
private:
	// INPUT PORTS
	static MObject CURVE;

	static MObject STRETCH;
	static MObject LENGTH;

	static MObject HANDLES;
	static MObject WORLD_MATRIX;
	static MObject TWIST;
	static MObject LOCAL_ROTATE;

	static MObject AIM_AXIS;
	static MObject UP_AXIS;

	static MObject OFFSETS;

	// OUTPUT PORTS
	static MObject OUT_MATRIX;


public:
	static MTypeId TYPE_ID;
	static MString TYPE_NAME;

	static constexpr double EPS{ 1.0e-5 };
	static const char* AXIS_TYPE[];

public:
	KNoFlipSplineSolver();
	virtual ~KNoFlipSplineSolver() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;
	static  void* creator();
	static  MStatus initialize();
	SchedulingType schedulingType() const override { return SchedulingType::kParallel; }
	static MMatrix solveNextMatrix(const MVector& prevV1,
								   const MVector& curV1,
								   const MMatrix& prevMatrix);


	static std::vector<Segment>  createMainData(const std::vector<double>& handleParams,
		const std::vector<MMatrix>& matrices,
		const std::vector<double>& outputParams,
		const MFnNurbsCurve& curveFn
	);

	static void setupUI();



};
