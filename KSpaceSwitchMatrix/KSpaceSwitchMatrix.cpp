#include "KSpaceSwitchMatrix.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MDataHandle.h>

#include <maya/MMatrixArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MTransformationMatrix.h>

#include <cmath>

MTypeId KSpaceSwitchMatrix::TYPE_ID{ 0x00141B96 };
MString KSpaceSwitchMatrix::TYPE_NAME{ "kSpaceSwitchMatrix" };

// INPUTS
MObject KSpaceSwitchMatrix::PARENT_INVERSE_MATRIX;

MObject KSpaceSwitchMatrix::TARGET;
MObject KSpaceSwitchMatrix::WEIGHT;
MObject KSpaceSwitchMatrix::TRANSLATE_ENABLE;
MObject KSpaceSwitchMatrix::ROTATE_ENABLE;
MObject KSpaceSwitchMatrix::SCALE_ENABLE;
MObject KSpaceSwitchMatrix::TARGET_MATRIX;
MObject KSpaceSwitchMatrix::OFFSET_MATRIX;

// OUTPUTS
MObject KSpaceSwitchMatrix::OUTPUT_MATRIX;

KSpaceSwitchMatrix::KSpaceSwitchMatrix() :MPxNode() {}
KSpaceSwitchMatrix::~KSpaceSwitchMatrix() {}

void* KSpaceSwitchMatrix::creator() { return new KSpaceSwitchMatrix{}; }

MStatus KSpaceSwitchMatrix::initialize()
{
	MFnCompoundAttribute compoundAttr{};
	MFnMatrixAttribute	 matrixAttr{};
	MFnNumericAttribute  numericAttr{};


	KSpaceSwitchMatrix::PARENT_INVERSE_MATRIX = matrixAttr.create("parentInverseMatrix", "pim", MFnMatrixAttribute::kDouble);
	matrixAttr.setReadable(false);
	matrixAttr.setStorable(true);
	matrixAttr.setKeyable(true);

	KSpaceSwitchMatrix::WEIGHT = numericAttr.create("weight", "w", MFnNumericData::kDouble, 1.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KSpaceSwitchMatrix::TRANSLATE_ENABLE = numericAttr.create("translateEnable", "te", MFnNumericData::kBoolean, true);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KSpaceSwitchMatrix::ROTATE_ENABLE = numericAttr.create("rotateEnable", "re", MFnNumericData::kBoolean, true);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KSpaceSwitchMatrix::SCALE_ENABLE = numericAttr.create("scaleEnable", "se", MFnNumericData::kBoolean, true);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KSpaceSwitchMatrix::TARGET_MATRIX = matrixAttr.create("targetMatrix", "tm", MFnMatrixAttribute::kDouble);
	matrixAttr.setKeyable(true);
	matrixAttr.setReadable(false);
	matrixAttr.setStorable(true);

	KSpaceSwitchMatrix::OFFSET_MATRIX = matrixAttr.create("offsetMatrix", "om", MFnMatrixAttribute::kDouble);
	matrixAttr.setKeyable(true);
	matrixAttr.setReadable(false);
	matrixAttr.setStorable(true);

	KSpaceSwitchMatrix::TARGET = compoundAttr.create("target", "t");
	compoundAttr.setArray(true);
	compoundAttr.setKeyable(true);
	compoundAttr.setReadable(false);
	compoundAttr.addChild(KSpaceSwitchMatrix::WEIGHT);
	compoundAttr.addChild(KSpaceSwitchMatrix::TRANSLATE_ENABLE);
	compoundAttr.addChild(KSpaceSwitchMatrix::ROTATE_ENABLE);
	compoundAttr.addChild(KSpaceSwitchMatrix::SCALE_ENABLE);
	compoundAttr.addChild(KSpaceSwitchMatrix::TARGET_MATRIX);
	compoundAttr.addChild(KSpaceSwitchMatrix::OFFSET_MATRIX);

	KSpaceSwitchMatrix::OUTPUT_MATRIX = matrixAttr.create("outputMatrix", "outm", MFnMatrixAttribute::kDouble);
	matrixAttr.setWritable(false);

	KSpaceSwitchMatrix::addAttribute(KSpaceSwitchMatrix::PARENT_INVERSE_MATRIX);
	KSpaceSwitchMatrix::addAttribute(KSpaceSwitchMatrix::TARGET);
	KSpaceSwitchMatrix::addAttribute(KSpaceSwitchMatrix::OUTPUT_MATRIX);

	KSpaceSwitchMatrix::attributeAffects(KSpaceSwitchMatrix::PARENT_INVERSE_MATRIX, KSpaceSwitchMatrix::OUTPUT_MATRIX);
	KSpaceSwitchMatrix::attributeAffects(KSpaceSwitchMatrix::TARGET,			    KSpaceSwitchMatrix::OUTPUT_MATRIX);

	KSpaceSwitchMatrix::setupUI();
	return MS::kSuccess;
}

void KSpaceSwitchMatrix::setupUI()
{

	const char* melCommand = R"mel(
    global proc AEkSpaceSwitchMatrixTemplate(string $nodeName)
    {
        editorTemplate -beginScrollLayout;
            editorTemplate -beginLayout "Target Matrix" -collapse 0;
                    editorTemplate -addControl "parentInverseMatrix";
                    editorTemplate -addControl "target";
            editorTemplate -endLayout;
                
            editorTemplate -beginLayout "Output Matrix" -collapse 0;    
                editorTemplate -addControl "outputMatrix";
            editorTemplate -endLayout;
    
        AEdependNodeTemplate $nodeName;
        editorTemplate -addExtraControls;
        editorTemplate -endScrollLayout;
    }
    )mel";

	MGlobal::executeCommandOnIdle(melCommand);
}

TransformComponents KSpaceSwitchMatrix::decomposeMatrix(const MMatrix& matrix)
{
	/*
	MVector translation(matrix[3][0], matrix[3][1], matrix[3][2]);
	
	MVector scale(std::sqrt(matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1] + matrix[0][2] * matrix[0][2]), 
				  std::sqrt(matrix[1][0] * matrix[1][0] + matrix[1][1] * matrix[1][1] + matrix[1][2] * matrix[1][2]), 
			      std::sqrt(matrix[2][0] * matrix[2][0] + matrix[2][1] * matrix[2][1] + matrix[2][2] * matrix[2][2]));

	MQuaternion rotation;
	rotation = matrix;
	return TransformComponents{ translation, rotation, scale };
	*/
	

	MTransformationMatrix tm{ matrix };

	MVector translation = tm.getTranslation(MSpace::kTransform);

	double scaleArray[3];
	tm.getScale(scaleArray, MSpace::kTransform);
	MVector scale{ scaleArray };

	MQuaternion rotation = tm.rotation();

	return TransformComponents{ translation, rotation, scale };
}

MMatrix KSpaceSwitchMatrix::blendMatrix(const MMatrix& m1, const MMatrix& m2, double weight)
{
	TransformComponents tc1 = KSpaceSwitchMatrix::decomposeMatrix(m1);
	TransformComponents tc2 = KSpaceSwitchMatrix::decomposeMatrix(m2);

	MVector translation	= KSpaceSwitchMatrix::blend<MVector>(tc1.translation, tc2.translation, weight);
	MMatrix rotMatrix   = slerp(tc1.rotation, tc2.rotation, weight).asMatrix();
	MVector scale	    = KSpaceSwitchMatrix::blend<MVector>(tc1.scale, tc2.scale, weight);

	rotMatrix[3][0] = translation[0];
	rotMatrix[3][1] = translation[1];
	rotMatrix[3][2] = translation[2];

	double sx = scale[0], sy = scale[1], sz = scale[2];

	rotMatrix[0][0] *= sx; rotMatrix[0][1] *= sx; rotMatrix[0][2] *= sx;
	rotMatrix[1][0] *= sy; rotMatrix[1][1] *= sy; rotMatrix[1][2] *= sy;
	rotMatrix[2][0] *= sz; rotMatrix[2][1] *= sz; rotMatrix[2][2] *= sz;

	return rotMatrix;
}

MStatus KSpaceSwitchMatrix::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	if (plug != KSpaceSwitchMatrix::OUTPUT_MATRIX)
		return MS::kUnknownParameter;

	MDataHandle		 outMatrixHandle = dataBlock.outputValue(KSpaceSwitchMatrix::OUTPUT_MATRIX);
	MArrayDataHandle targetHandle	 = dataBlock.inputArrayValue(KSpaceSwitchMatrix::TARGET);
	unsigned int	 numtargetHandle = targetHandle.elementCount();

	if (numtargetHandle < 1)
	{
		outMatrixHandle.setMMatrix(MMatrix{});
		dataBlock.setClean(plug);
		return MS::kSuccess;
	}

	MMatrix	parentInverseMatrix = dataBlock.inputValue(KSpaceSwitchMatrix::PARENT_INVERSE_MATRIX).asMatrix();

	MMatrixArray matrices;
	MDoubleArray weights;
	matrices.setSizeIncrement(numtargetHandle);
	weights.setSizeIncrement(numtargetHandle);
	double totalWeight{ 0.0 };

	for (int index{ 0 }; index < numtargetHandle; ++index)
	{
		targetHandle.jumpToArrayElement(index);
		MDataHandle elementHandle = targetHandle.inputValue();
		double		weight		  = elementHandle.child(KSpaceSwitchMatrix::WEIGHT).asDouble();

		if (weight < KSpaceSwitchMatrix::EPS)
			continue;

		MMatrix targetMatrix = elementHandle.child(KSpaceSwitchMatrix::TARGET_MATRIX).asMatrix();
		MMatrix offsetMatrix = elementHandle.child(KSpaceSwitchMatrix::OFFSET_MATRIX).asMatrix();
		MMatrix outMatrix    = offsetMatrix * targetMatrix * parentInverseMatrix;

		if (not elementHandle.child(KSpaceSwitchMatrix::TRANSLATE_ENABLE).asBool())
		{
			outMatrix[3][0] = 0.0; outMatrix[3][1] = 0.0; outMatrix[3][2] = 0.0;
		}

		if (not elementHandle.child(KSpaceSwitchMatrix::ROTATE_ENABLE).asBool())
		{
			double sx = std::sqrt(outMatrix[0][0] * outMatrix[0][0] + outMatrix[0][1] * outMatrix[0][1] + outMatrix[0][2] * outMatrix[0][2]);
			double sy = std::sqrt(outMatrix[1][0] * outMatrix[1][0] + outMatrix[1][1] * outMatrix[1][1] + outMatrix[1][2] * outMatrix[1][2]);
			double sz = std::sqrt(outMatrix[2][0] * outMatrix[2][0] + outMatrix[2][1] * outMatrix[2][1] + outMatrix[2][2] * outMatrix[2][2]);

			outMatrix[0][0] = sx;  outMatrix[0][1] = 0.0; outMatrix[0][2] = 0.0;
			outMatrix[1][0] = 0.0; outMatrix[1][1] = sy;  outMatrix[1][2] = 0.0;
			outMatrix[2][0] = 0.0; outMatrix[2][1] = 0.0; outMatrix[2][2] = sz;
		}

		if (not elementHandle.child(KSpaceSwitchMatrix::SCALE_ENABLE).asBool())
		{
			// v1
			double v1x = outMatrix[0][0], v1y = outMatrix[0][1], v1z = outMatrix[0][2];
			double v1SqLength = v1x * v1x + v1y * v1y + v1z * v1z;

			if (v1SqLength > 1.0e-12)
			{
				double v1InvLength = 1.0 / std::sqrt(v1SqLength);
				outMatrix[0][0] *= v1InvLength; outMatrix[0][1] *= v1InvLength; outMatrix[0][2] *= v1InvLength;
			}

			// v2
			double v2x = outMatrix[1][0], v2y = outMatrix[1][1], v2z = outMatrix[1][2];
			double v2SqLength = v2x * v2x + v2y * v2y + v2z * v2z;

			if (v2SqLength > 1.0e-12)
			{
				double v2InvLength = 1.0 / std::sqrt(v2SqLength);
				outMatrix[1][0] *= v2InvLength; outMatrix[1][1] *= v2InvLength; outMatrix[1][2] *= v2InvLength;
			}

			// v3
			double v3x = outMatrix[2][0], v3y = outMatrix[2][1], v3z = outMatrix[2][2];
			double v3SqLength = v3x * v3x + v3y * v3y + v3z * v3z;

			if (v3SqLength > 1.0e-12)
			{
				double v3InvLength = 1.0 / std::sqrt(v3SqLength);
				outMatrix[2][0] *= v3InvLength; outMatrix[2][1] *= v3InvLength; outMatrix[2][2] *= v3InvLength;
			}
		}

		matrices.append(outMatrix);
		weights.append(weight);
		totalWeight += weight;
	}

	if (matrices.length() == 0)
	{
		outMatrixHandle.setMMatrix(MMatrix{});
		dataBlock.setClean(plug);
		return MS::kSuccess;
	}

	if (matrices.length() == 1)
	{
		MMatrix outMatrix = (weights[0] > 1.0 - KSpaceSwitchMatrix::EPS) ? matrices[0] : KSpaceSwitchMatrix::blendMatrix(MMatrix{}, matrices[0], weights[0]);
		outMatrixHandle.setMMatrix(outMatrix);

		dataBlock.setClean(plug);
		return MS::kSuccess;
	}

	if (totalWeight < KSpaceSwitchMatrix::EPS)
	{
		outMatrixHandle.setMMatrix(MMatrix{});
		dataBlock.setClean(plug);
		return MS::kSuccess;
	}


	MVector		outOffset{ 0.0, 0.0, 0.0 };
	MVector		outScale{ 0.0, 0.0, 0.0 };
	MQuaternion outQuat{ 0.0, 0.0, 0.0, 0.0 }; 
	MQuaternion firstQuat;                    
	bool hasStarted{ false };

	for (unsigned int index = 0; index < weights.length(); ++index)
	{
		double w = weights[index] / totalWeight;
		TransformComponents tc = KSpaceSwitchMatrix::decomposeMatrix(matrices[index]);

		outOffset += tc.translation * w;
		outScale  += tc.scale	    * w;

		if (not hasStarted)
		{
			firstQuat = tc.rotation;
			hasStarted = true;
		}

		double dot = tc.rotation.x * firstQuat.x + tc.rotation.y * firstQuat.y + tc.rotation.z * firstQuat.z + tc.rotation.w * firstQuat.w;
		double mult = (dot >= 0.0) ? w : -w;

		outQuat.x += tc.rotation.x * mult;
		outQuat.y += tc.rotation.y * mult;
		outQuat.z += tc.rotation.z * mult;
		outQuat.w += tc.rotation.w * mult;
		
	}

	outQuat.normalizeIt();

	MMatrix outputMatrix = outQuat.asMatrix();

	// UPDATE MATRIX
	outputMatrix[3][0] = outOffset[0]; outputMatrix[3][1] = outOffset[1]; outputMatrix[3][2] = outOffset[2];
	double sx = outScale[0], sy = outScale[1], sz = outScale[2];
	outputMatrix[0][0] *= sx; outputMatrix[0][1] *= sx; outputMatrix[0][2] *= sx;
	outputMatrix[1][0] *= sy; outputMatrix[1][1] *= sy; outputMatrix[1][2] *= sy;
	outputMatrix[2][0] *= sz; outputMatrix[2][1] *= sz; outputMatrix[2][2] *= sz;

	outMatrixHandle.setMMatrix(outputMatrix);
	dataBlock.setClean(plug);
	return MS::kSuccess;
}
