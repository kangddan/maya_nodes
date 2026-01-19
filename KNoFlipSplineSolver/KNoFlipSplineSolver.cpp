#include "KNoFlipSplineSolver.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnData.h>
#include <maya/MAngle.h>
#include <maya/MGlobal.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h >
#include <maya/MTransformationMatrix.h >
#include <maya/MDoubleArray.h >
#include <maya/MQuaternion.h >

#include <cmath>


MTypeId KNoFlipSplineSolver::TYPE_ID{ 0x00141B94 };
MString KNoFlipSplineSolver::TYPE_NAME{ "kNoFlipSplineSolver" };

// INPUTS
MObject KNoFlipSplineSolver::CURVE;
MObject KNoFlipSplineSolver::STRETCH;
MObject KNoFlipSplineSolver::LENGTH;

MObject KNoFlipSplineSolver::HANDLES;
MObject KNoFlipSplineSolver::WORLD_MATRIX;
MObject KNoFlipSplineSolver::TWIST;
MObject KNoFlipSplineSolver::LOCAL_ROTATE;

MObject KNoFlipSplineSolver::AIM_AXIS;
MObject KNoFlipSplineSolver::UP_AXIS;
MObject KNoFlipSplineSolver::OFFSETS;

// OUTPUTS
MObject KNoFlipSplineSolver::OUT_MATRIX;

const char* KNoFlipSplineSolver::AXIS_TYPE[]{ "X", "Y", "Z", "-X", "-Y", "-Z" };

KNoFlipSplineSolver::KNoFlipSplineSolver() :MPxNode() {}
KNoFlipSplineSolver::~KNoFlipSplineSolver() {}

void* KNoFlipSplineSolver::creator() { return new KNoFlipSplineSolver{}; }


MStatus KNoFlipSplineSolver::initialize()
{
	MFnCompoundAttribute compoundAttr{};
	MFnMatrixAttribute	 matrixAttr{};
	MFnNumericAttribute  numericAttr{};
	MFnEnumAttribute	 enumAttr{};
	MFnTypedAttribute    typedAttr{};
	MFnUnitAttribute	 unitAttr{};


	KNoFlipSplineSolver::CURVE = typedAttr.create("curve", "c", MFnData::kNurbsCurve);
	typedAttr.setReadable(false);
	typedAttr.setWritable(true);
	typedAttr.setStorable(true);
	typedAttr.setCached(false);
	typedAttr.setKeyable(true);

	KNoFlipSplineSolver::STRETCH = numericAttr.create("stretch", "s", MFnNumericData::kDouble, 1.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KNoFlipSplineSolver::LENGTH = numericAttr.create("length", "l", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KNoFlipSplineSolver::AIM_AXIS = enumAttr.create("aimAxis", "aa", 0);
	for (short index{ 0 }; index < 6; ++index)
		enumAttr.addField(KNoFlipSplineSolver::AXIS_TYPE[index], index);
	enumAttr.setKeyable(true);
	enumAttr.setReadable(false);
	enumAttr.setStorable(true);

	KNoFlipSplineSolver::UP_AXIS = enumAttr.create("upAxis", "ua", 0);
	for (short index{ 0 }; index < 6; ++index)
		enumAttr.addField(KNoFlipSplineSolver::AXIS_TYPE[index], index);
	enumAttr.setKeyable(true);
	enumAttr.setReadable(false);
	enumAttr.setStorable(true);

	KNoFlipSplineSolver::WORLD_MATRIX = matrixAttr.create("worldMatrix", "wm", MFnMatrixAttribute::kDouble);
	matrixAttr.setKeyable(true);
	matrixAttr.setReadable(false);
	matrixAttr.setStorable(true);

	KNoFlipSplineSolver::TWIST = numericAttr.create("twist", "t", MFnNumericData::kBoolean, true);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KNoFlipSplineSolver::LOCAL_ROTATE = unitAttr.create("localRotate", "lr", MAngle(0.0, MAngle::kDegrees));
	unitAttr.setSoftMin(MAngle(-360, MAngle::kDegrees));
	unitAttr.setSoftMax(MAngle(360, MAngle::kDegrees));
	unitAttr.setKeyable(true);
	unitAttr.setReadable(true);
	unitAttr.setStorable(true);

	KNoFlipSplineSolver::HANDLES = compoundAttr.create("handles", "h");
	compoundAttr.setArray(true);
	compoundAttr.setKeyable(true);
	compoundAttr.setReadable(false);
	compoundAttr.addChild(KNoFlipSplineSolver::WORLD_MATRIX);
	compoundAttr.addChild(KNoFlipSplineSolver::TWIST);
	compoundAttr.addChild(KNoFlipSplineSolver::LOCAL_ROTATE);

	KNoFlipSplineSolver::OFFSETS = numericAttr.create("offsets", "offs", MFnNumericData::kDouble, 0.0);
	numericAttr.setDefault(-1.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setArray(true);
	numericAttr.setUsesArrayDataBuilder(true);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KNoFlipSplineSolver::OUT_MATRIX = matrixAttr.create("outMatrix", "outM", MFnMatrixAttribute::kDouble);
	matrixAttr.setArray(true);
	matrixAttr.setUsesArrayDataBuilder(true);
	matrixAttr.setWritable(false);

	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::CURVE);
	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::LENGTH);
	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::STRETCH);
	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::AIM_AXIS);
	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::UP_AXIS);

	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::HANDLES);
	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::OFFSETS);
	KNoFlipSplineSolver::addAttribute(KNoFlipSplineSolver::OUT_MATRIX);

	std::vector<MObject> inPlugs{ KNoFlipSplineSolver::CURVE,    KNoFlipSplineSolver::LENGTH,  KNoFlipSplineSolver::STRETCH,
								  KNoFlipSplineSolver::AIM_AXIS, KNoFlipSplineSolver::UP_AXIS, KNoFlipSplineSolver::HANDLES,
								  KNoFlipSplineSolver::OFFSETS };

	for (const MObject& inPlug : inPlugs)
		KNoFlipSplineSolver::attributeAffects(inPlug, KNoFlipSplineSolver::OUT_MATRIX);

	KNoFlipSplineSolver::setupUI();
	return MS::kSuccess;
}

void KNoFlipSplineSolver::setupUI()
{

	const char* melCommand = R"mel(
    global proc AEkNoFlipSplineSolverTemplate(string $nodeName)
    {
        editorTemplate -beginScrollLayout;

            editorTemplate -beginLayout "Main" -collapse 0;
			    editorTemplate -callCustom ("AEinputNew \"In Curve\"") 
                                           ("AEinputReplace \"In Curve\"") 
                                            "curve"; 
                editorTemplate -addSeparator;
                editorTemplate -addControl "length";
                editorTemplate -addControl "stretch";
                    
                editorTemplate -addControl "aimAxis";
                editorTemplate -addControl "upAxis";
                editorTemplate -addControl "handles";
                editorTemplate -addControl "offsets";
                    
                editorTemplate -addControl "OUT_MATRIX";
            editorTemplate -endLayout;
    
            AEdependNodeTemplate $nodeName;
            editorTemplate -addExtraControls;
        editorTemplate -endScrollLayout;
    }
    )mel";

	MGlobal::executeCommandOnIdle(melCommand);
}


MMatrix KNoFlipSplineSolver::solveNextMatrix(const MVector& prevV1,
	const MVector& curV1,
	const MMatrix& prevMatrix)
{
	MVector axis = curV1 ^ prevV1;
	double  angle = curV1.angle(prevV1);

	MTransformationMatrix transformMatrix{ prevMatrix };
	transformMatrix.setToRotationAxis(axis, angle);

	MMatrix rotMatrix = transformMatrix.asMatrix();

	MVector oldV2 = (rotMatrix * MVector{ prevMatrix[1][0], prevMatrix[1][1], prevMatrix[1][2] });
	oldV2.normalize();

	MVector v3 = curV1 ^ oldV2;
	MVector v2 = v3 ^ curV1;

	double _m[4][4] = { { curV1[0],   curV1[1],  curV1[2],  0.0 },
						{ v2[0],	  v2[1],  v2[2],  0.0 },
						{ v3[0],   v3[1],  v3[2],	0.0 },
						{ 0.0,		  0.0,		 0.0,		1.0 } };

	return MMatrix{ _m };
}


std::vector<Segment> KNoFlipSplineSolver::createMainData(const std::vector<double>& handleParams,
	const std::vector<MMatrix>& matrices,
	const std::vector<double>& outputParams,
	const MFnNurbsCurve& curveFn)
{
	int numSpans = static_cast<int>(handleParams.size()) - 1;

	std::vector<Segment> segments;
	segments.reserve(numSpans);

	for (int index{ 0 }; index < numSpans; ++index)
		segments.emplace_back(matrices[index], matrices[index + 1]);

	for (double outParam : outputParams)
	{
		int	   targetIndex{ -1 };
		double blendWeight{ 0.0 };

		if (outParam <= handleParams.front())
		{
			targetIndex = 0;
			blendWeight = 0.0;
		}
		else if (outParam >= handleParams.back())
		{
			targetIndex = numSpans - 1;
			blendWeight = 1.0;
		}
		else
		{
			for (int index{ 0 }; index < numSpans; ++index)
			{
				double start = handleParams[index];
				double end = handleParams[index + 1];

				if (start <= outParam and outParam <= end)
				{
					targetIndex = index;
					double range = end - start;
					blendWeight = (range > KNoFlipSplineSolver::EPS) ? (outParam - start) / range : 0.0;
					break;
				}
			}
		}

		if (targetIndex != -1)
		{
			MPoint point;
			curveFn.getPointAtParam(outParam, point, MSpace::kObject);
			MVector tangent = curveFn.tangent(outParam, MSpace::kObject);
			tangent.normalize();

			Segment& segment = segments[targetIndex];
			segment.points.emplace_back(point);
			segment.tangents.emplace_back(tangent);
			segment.blendWeights.emplace_back(blendWeight);
		}
	}

	return segments;
}


MStatus KNoFlipSplineSolver::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	if (plug != KNoFlipSplineSolver::OUT_MATRIX)
		return MS::kUnknownParameter;

	MObject curveObj = dataBlock.inputValue(KNoFlipSplineSolver::CURVE).asNurbsCurve(); // get curve local space

	if (not curveObj.hasFn(MFn::kNurbsCurve))
		return MS::kSuccess;

	MArrayDataHandle curveHandle = dataBlock.inputArrayValue(KNoFlipSplineSolver::HANDLES);
	unsigned int     numCvHandle = curveHandle.elementCount();

	if (numCvHandle < 2)
		return MS::kSuccess;

	MArrayDataHandle offsetsHandle = dataBlock.inputArrayValue(KNoFlipSplineSolver::OFFSETS);
	unsigned int     numOffHandle = offsetsHandle.elementCount();
	if (numOffHandle < 2)
		return MS::kSuccess;

	// 1 Get Joint Weights
	std::vector<double> offsets;
	offsets.reserve(numOffHandle);

	for (unsigned int index{ 0 }; index < numOffHandle; ++index)
	{
		offsets.emplace_back(offsetsHandle.inputValue().asDouble());
		offsetsHandle.next();
	}

	// 2 Get Curve Data
	MFnNurbsCurve curveFn{ curveObj };

	int    cvCount = curveFn.numCVs();
	double length = curveFn.length();

	MDoubleArray knots{};
	curveFn.getKnots(knots);

	int degree = curveFn.degree();

	// 3 Calculate mathematical parameter for each CV using Greville Abscissae

	std::vector<double> _cvParams;
	_cvParams.reserve(cvCount);

	double invDegree = 1.0 / static_cast<double>(degree);

	for (int i = 0; i < cvCount; ++i)
	{
		double sumknots = 0.0;
		for (int _i{ i }; _i < i + degree; ++_i)
			sumknots += knots[_i];
		_cvParams.emplace_back(sumknots * invDegree);
	}

	// Remap handle index to CV index to handle cases where handle count 
	// does not match CV count (e.g., when one joint influences multiple CVs via skinning!)

	std::vector<double> handleParams;
	handleParams.reserve(numCvHandle);

	for (int index{ 0 }; index < numCvHandle; ++index)
	{
		int cvIndex = static_cast<int>(std::round((static_cast<double>(index) / (numCvHandle - 1)) * (cvCount - 1)));
		handleParams.emplace_back(_cvParams[cvIndex]);
	}

	// 5 Create uniformParams

	double _baseLength = dataBlock.inputValue(KNoFlipSplineSolver::LENGTH).asDouble();
	double baseLength = (_baseLength < KNoFlipSplineSolver::EPS) ? length : _baseLength;
	double stretch = dataBlock.inputValue(KNoFlipSplineSolver::STRETCH).asDouble();

	std::vector<double> outputParams;
	outputParams.reserve(numOffHandle);

	for (double off : offsets)
	{
		double fixedPos{ baseLength * off };
		double stretchPos{ length * off };

		double mixedPos{ fixedPos + (stretchPos - fixedPos) * stretch };
		double safePos{ std::max(0.0, std::min(mixedPos, length)) };
		outputParams.emplace_back(curveFn.findParamFromLength(safePos));
	}

	// 6 Extract matrices and states from handle array

	std::vector<MMatrix> matrices;
	matrices.reserve(numCvHandle);

	//std::vector<bool> twistStates;
	//twistStates.reserve(numCvHandle);

	for (unsigned int index{ 0 }; index < numCvHandle; ++index)
	{
		MDataHandle elementHandle = curveHandle.inputValue();

		// Normalize the controller matrix by resetting scale to (1,1,1) 
		// to prevent normalization errors(NaN) in Parallel Transport calculations
		MMatrix handleMatrix = elementHandle.child(KNoFlipSplineSolver::WORLD_MATRIX).asMatrix();
		MTransformationMatrix transformMatrix{ handleMatrix };
		double scale[3]{ 1.0, 1.0, 1.0 };
		transformMatrix.setScale(scale, MSpace::kWorld);

		matrices.emplace_back(transformMatrix.asMatrix());
		//twistStates.emplace_back(elementHandle.child(KNoFlipSplineSolver::TWIST).asBool());
		curveHandle.next();
	}

	std::vector<Segment> mainData = KNoFlipSplineSolver::createMainData(handleParams, matrices, outputParams, curveFn);
	std::vector<MMatrix> newMatrices;
	newMatrices.reserve(numOffHandle);

	for (const Segment& segment : mainData)
	{
		const MMatrix& startMatrix = segment.startMatrix;
		const MMatrix& endMatrix = segment.endMatrix;

		const std::vector<MVector>& tangents = segment.tangents;
		const std::vector<MPoint>& points = segment.points;
		const std::vector<double>& blendWeights = segment.blendWeights;

		int numTangents = static_cast<int>(tangents.size());

		std::vector<MMatrix> startMatrices;
		startMatrices.reserve(numTangents);

		MMatrix startCacheMatrix{ startMatrix };
		MVector startCacheV1{ startCacheMatrix[0][0], startCacheMatrix[0][1], startCacheMatrix[0][2] };

		for (const MVector& tangent : tangents)
		{
			if (tangent.isParallel(startCacheV1, KNoFlipSplineSolver::EPS))
			{
				startMatrices.emplace_back(MMatrix{ startCacheMatrix });
			}
			else
			{
				MMatrix nextMatrix = KNoFlipSplineSolver::solveNextMatrix(startCacheV1, tangent, startCacheMatrix);
				startMatrices.emplace_back(nextMatrix);
				startCacheV1 = tangent;
				startCacheMatrix = nextMatrix;
			}
		}

		//  Propagate rotation from end controller back up the curve
		std::vector<MMatrix> endMatrices;
		MMatrix endCacheMatrix{ endMatrix };
		MVector endCacheV1{ endCacheMatrix[0][0], endCacheMatrix[0][1], endCacheMatrix[0][2] };

		for (auto it{ tangents.rbegin() }; it != tangents.rend(); ++it)
		{
			const MVector& tangent = *it;
			if (tangent.isParallel(endCacheV1, KNoFlipSplineSolver::EPS))
			{
				endMatrices.emplace_back(MMatrix{ endCacheMatrix });
			}
			else
			{
				MMatrix nextMatrix = KNoFlipSplineSolver::solveNextMatrix(endCacheV1, tangent, endCacheMatrix);
				endMatrices.emplace_back(nextMatrix);
				endCacheV1 = tangent;
				endCacheMatrix = nextMatrix;
			}
		}

		std::reverse(endMatrices.begin(), endMatrices.end());

		// Blend forward and backward passes and inject curve point positions
		int index{ 0 };
		for (const double weight : blendWeights)
		{
			MQuaternion quat1{};
			MQuaternion quat2{};
			
			quat1 = startMatrices[index];
			quat2 = endMatrices[index];

			MMatrix outMatrix = slerp(quat1, quat2, weight).asMatrix();
			const MPoint& point = points[index];

			outMatrix[3][0] = point.x;
			outMatrix[3][1] = point.y;
			outMatrix[3][2] = point.z;
			newMatrices.emplace_back(outMatrix);
			++index;
		}
	}


	MArrayDataHandle  outMatrixHandle = dataBlock.outputArrayValue(KNoFlipSplineSolver::OUT_MATRIX);
	MArrayDataBuilder outMatrixBuilder = outMatrixHandle.builder();

	int index{ 0 };
	for (const MMatrix& matrix : newMatrices)
	{
		//MDataHandle outElementHandle = outMatrixBuilder.addLast();
		//outElementHandle.setMMatrix(matrix);

		MDataHandle outElementHandle = outMatrixBuilder.addElement(index);
		outElementHandle.setMMatrix(matrix);
		++index;
	}

	outMatrixHandle.set(outMatrixBuilder);
	outMatrixHandle.setAllClean();

	dataBlock.setClean(plug);
	return MS::kSuccess;

};
