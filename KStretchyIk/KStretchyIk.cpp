#include "KStretchyIk.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <vector>
#include <cmath>
#include <limits>

MTypeId KStretchyIk::TYPE_ID  {0x00141B91};
MString KStretchyIk::TYPE_NAME{ "kStretchyIk" };
double  KStretchyIk::MAIN_EPS { 1.0e-5 };

// INPUT PORTS
MObject KStretchyIk::REST_UPR_LENGTH;
MObject KStretchyIk::REST_LWR_LENGTH;

MObject KStretchyIk::MULT_UPR_LENGTH;
MObject KStretchyIk::MULT_LWR_LENGTH;
MObject KStretchyIk::STRETCH;
MObject KStretchyIk::SOFT;
MObject KStretchyIk::SLIDE;
MObject KStretchyIk::PIN;

MObject KStretchyIk::ROOT_MATRIX;
MObject KStretchyIk::POLE_MATRIX;
MObject KStretchyIk::GOAL_MATRIX;
MObject KStretchyIk::INVERT;

// OUTPUT PORTS
MObject KStretchyIk::OUT_UPR_LENGTH;
MObject KStretchyIk::OUT_LWR_LENGTH;
MObject KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX;


KStretchyIk::KStretchyIk () :MPxNode() {}
KStretchyIk::~KStretchyIk() {}

void* KStretchyIk::creator() { return new KStretchyIk(); }


MStatus KStretchyIk::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	if (not (plug == KStretchyIk::OUT_UPR_LENGTH or
			 plug == KStretchyIk::OUT_LWR_LENGTH or
			 plug == KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX))
	{
		return MS::kUnknownParameter;
	}

	double invertMult = dataBlock.inputValue(KStretchyIk::INVERT).asBool() ? - 1.0 : 1.0;

	double rawRestUpr = std::fabs(dataBlock.inputValue(KStretchyIk::REST_UPR_LENGTH).asDouble());
	double rawRestLwr = std::fabs(dataBlock.inputValue(KStretchyIk::REST_LWR_LENGTH).asDouble());

	double uprMultiplier = dataBlock.inputValue(KStretchyIk::MULT_UPR_LENGTH).asDouble();
	double lwrMultiplier = dataBlock.inputValue(KStretchyIk::MULT_LWR_LENGTH).asDouble();

	double stretchWeight = dataBlock.inputValue(KStretchyIk::STRETCH).asDouble();
	double softRatio	 = dataBlock.inputValue(KStretchyIk::SOFT)	 .asDouble();
	double slideValue	 = dataBlock.inputValue(KStretchyIk::SLIDE)	 .asDouble();
	double pinWeight	 = dataBlock.inputValue(KStretchyIk::PIN)	 .asDouble();

	MMatrix rootMatrix = dataBlock.inputValue(KStretchyIk::ROOT_MATRIX).asMatrix();
	MMatrix poleMatrix = dataBlock.inputValue(KStretchyIk::POLE_MATRIX).asMatrix();
	MMatrix goalMatrix = dataBlock.inputValue(KStretchyIk::GOAL_MATRIX).asMatrix();

	MVector rootPos{ rootMatrix[3][0], rootMatrix[3][1], rootMatrix[3][2] };
	MVector polePos{ poleMatrix[3][0], poleMatrix[3][1], poleMatrix[3][2] };
	MVector goalPos{ goalMatrix[3][0], goalMatrix[3][1], goalMatrix[3][2] };

	MVector scaleVec{ rootMatrix[1][0], rootMatrix[1][1], rootMatrix[1][2] };
	double globalScale = scaleVec.length();

	double baseUprLen = rawRestUpr * uprMultiplier;
	double baseLwrLen = rawRestLwr * lwrMultiplier;

	double slideUprLen  = baseUprLen * (slideValue + 1.0);
	double slideLwrLen  = baseLwrLen * (1.0 - slideValue);
	double totalRestLen = slideUprLen + slideLwrLen;

	double softThreshold   = totalRestLen * softRatio;
	double softMinBound    = std::max(softThreshold, KStretchyIk::MAIN_EPS);
	double softTriggerDist = totalRestLen - softThreshold;

	double distRootToGoal  = (goalPos - rootPos).length() / globalScale;
	
	double softDiff = (softTriggerDist - distRootToGoal) / softMinBound;
	double softExp  = (softDiff > 700.0) ? std::numeric_limits<double>::infinity() : std::exp(softDiff);

	double adjustedDist = (distRootToGoal <= softTriggerDist) ? 
						   distRootToGoal :
						   totalRestLen - (softExp * softMinBound);

	double stretchFactor = (distRootToGoal > adjustedDist) ?
							distRootToGoal / adjustedDist :
							1.0;

	double stretchedUprLen = KStretchyIk::mix(slideUprLen, stretchFactor * slideUprLen, stretchWeight);
	double stretchedLwrLen = KStretchyIk::mix(slideLwrLen, stretchFactor * slideLwrLen, stretchWeight);

	double distRootToPole = (polePos - rootPos).length() / globalScale;
	double distGoalToPole = (polePos - goalPos).length() / globalScale;

	double finalUprLen = KStretchyIk::mix(stretchedUprLen, distRootToPole, pinWeight) * invertMult;
	double finalLwrLen = KStretchyIk::mix(stretchedLwrLen, distGoalToPole, pinWeight) * invertMult;

	MVector aimDir	    = (goalPos - rootPos).normal();
	double goalMixDist  = KStretchyIk::mix(adjustedDist, distRootToGoal, stretchWeight) * globalScale;
	MVector ikHandlePos = KStretchyIk::mix(rootPos + (aimDir * goalMixDist), goalPos, pinWeight);

	MMatrix ikWorldMatrix;
	ikWorldMatrix[3][0] = ikHandlePos.x;
	ikWorldMatrix[3][1] = ikHandlePos.y;
	ikWorldMatrix[3][2] = ikHandlePos.z;

	MMatrix ikLocalMatrix = ikWorldMatrix * goalMatrix.inverse();

	dataBlock.outputValue(KStretchyIk::OUT_UPR_LENGTH).setDouble(finalUprLen);
	dataBlock.outputValue(KStretchyIk::OUT_LWR_LENGTH).setDouble(finalLwrLen);
	dataBlock.outputValue(KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX).setMMatrix(ikLocalMatrix);

	dataBlock.setClean(KStretchyIk::OUT_UPR_LENGTH);
	dataBlock.setClean(KStretchyIk::OUT_LWR_LENGTH);
	dataBlock.setClean(KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX);

	return MS::kSuccess;
}


MStatus KStretchyIk::initialize()
{
	MFnMatrixAttribute  matrixAttr;
	MFnNumericAttribute numericAttr;

	// ADD INPUT PORTS
	KStretchyIk::REST_UPR_LENGTH = numericAttr.create("restUprLength", "rul", MFnNumericData::kDouble, KStretchyIk::MAIN_EPS);
	numericAttr.setKeyable(false);
	numericAttr.setStorable(true);
	numericAttr.setReadable(false);

	KStretchyIk::REST_LWR_LENGTH = numericAttr.create("restLwrLength", "rll", MFnNumericData::kDouble, KStretchyIk::MAIN_EPS);
	numericAttr.setKeyable(false);
	numericAttr.setStorable(true);
	numericAttr.setReadable(false);

	KStretchyIk::MULT_UPR_LENGTH = numericAttr.create("multUprLength", "mul", MFnNumericData::kDouble, 1.0);
	numericAttr.setMin(KStretchyIk::MAIN_EPS);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setReadable(true);

	KStretchyIk::MULT_LWR_LENGTH = numericAttr.create("multLwrLength", "mll", MFnNumericData::kDouble, 1.0);
	numericAttr.setMin(KStretchyIk::MAIN_EPS);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setReadable(true);

	KStretchyIk::STRETCH = numericAttr.create("stretch", "st", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setReadable(true);

	KStretchyIk::SOFT = numericAttr.create("soft", "sf", MFnNumericData::kDouble, KStretchyIk::MAIN_EPS);
	numericAttr.setMin(KStretchyIk::MAIN_EPS);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setReadable(true);

	KStretchyIk::SLIDE = numericAttr.create("slide", "sl", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(-1.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setReadable(true);

	KStretchyIk::PIN = numericAttr.create("pin", "pn", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setReadable(true);

	KStretchyIk::INVERT = numericAttr.create("invert", "iv", MFnNumericData::kBoolean, false);
	numericAttr.setKeyable(false);
	numericAttr.setStorable(true);
	numericAttr.setReadable(false);

	KStretchyIk::ROOT_MATRIX = matrixAttr.create("rootMatrix", "rm", MFnMatrixAttribute::kDouble);
	matrixAttr.setKeyable(true);
	matrixAttr.setStorable(true);
	matrixAttr.setReadable(false);

	KStretchyIk::POLE_MATRIX = matrixAttr.create("poleMatrix", "pm", MFnMatrixAttribute::kDouble);
	matrixAttr.setKeyable(true);
	matrixAttr.setStorable(true);
	matrixAttr.setReadable(false);

	KStretchyIk::GOAL_MATRIX = matrixAttr.create("goalMatrix", "gm", MFnMatrixAttribute::kDouble);
	matrixAttr.setKeyable(true);
	matrixAttr.setStorable(true);
	matrixAttr.setReadable(false);

	// ADD OUTPUT PORTS
	KStretchyIk::OUT_UPR_LENGTH = numericAttr.create("outUprLength", "oul", MFnNumericData::kDouble, 0.0);
	numericAttr.setWritable(false);
	numericAttr.setKeyable(false);

	KStretchyIk::OUT_LWR_LENGTH = numericAttr.create("outLwrLength", "oll", MFnNumericData::kDouble, 0.0);
	numericAttr.setWritable(false);
	numericAttr.setKeyable(false);

	KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX = matrixAttr.create("outIkHandleLocalMatrix", "oihlm", MFnMatrixAttribute::kDouble);
	matrixAttr.setWritable(false);
	matrixAttr.setKeyable(false);

	// ADD INPUT PORTS
	KStretchyIk::addAttribute(KStretchyIk::REST_UPR_LENGTH);
	KStretchyIk::addAttribute(KStretchyIk::REST_LWR_LENGTH);
	KStretchyIk::addAttribute(KStretchyIk::INVERT);

	KStretchyIk::addAttribute(KStretchyIk::MULT_UPR_LENGTH);
	KStretchyIk::addAttribute(KStretchyIk::MULT_LWR_LENGTH);
	KStretchyIk::addAttribute(KStretchyIk::STRETCH);
	KStretchyIk::addAttribute(KStretchyIk::SOFT);
	KStretchyIk::addAttribute(KStretchyIk::SLIDE);
	KStretchyIk::addAttribute(KStretchyIk::PIN);

	KStretchyIk::addAttribute(KStretchyIk::ROOT_MATRIX);
	KStretchyIk::addAttribute(KStretchyIk::POLE_MATRIX);
	KStretchyIk::addAttribute(KStretchyIk::GOAL_MATRIX);

	// ADD OUTPUT PORTS
	KStretchyIk::addAttribute(KStretchyIk::OUT_UPR_LENGTH);
	KStretchyIk::addAttribute(KStretchyIk::OUT_LWR_LENGTH);
	KStretchyIk::addAttribute(KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX);

	std::vector<MObject> inPlugs  { KStretchyIk::REST_UPR_LENGTH, KStretchyIk::REST_LWR_LENGTH, KStretchyIk::INVERT,
								    KStretchyIk::MULT_UPR_LENGTH, KStretchyIk::MULT_LWR_LENGTH, KStretchyIk::STRETCH, 
								    KStretchyIk::SOFT,			  KStretchyIk::SLIDE,		    KStretchyIk::PIN,
								    KStretchyIk::ROOT_MATRIX,	  KStretchyIk::POLE_MATRIX,	    KStretchyIk::GOAL_MATRIX };

	std::vector<MObject> outPlugs { KStretchyIk::OUT_UPR_LENGTH,  KStretchyIk::OUT_LWR_LENGTH,  KStretchyIk::OUT_IK_HANDLE_LOCAL_MATRIX };
	
	for (const MObject& inPlug : inPlugs)
	{
		for (const MObject& outPlug : outPlugs)
			KStretchyIk::attributeAffects(inPlug, outPlug);
	}
		
	KStretchyIk::setupUI();
	return MStatus::kSuccess;
}

void KStretchyIk::setupUI()
{
	const char* melCommand = R"(
	global proc AEkStretchyIkTemplate(string $nodeName)
	{
		editorTemplate -beginScrollLayout;

			editorTemplate -beginLayout "Rest Lengths" -collapse 0;
				editorTemplate -addControl "restUprLength";
				editorTemplate -addControl "restLwrLength";
			editorTemplate -endLayout;

			editorTemplate -beginLayout "Stretch Controls" -collapse 0;
				editorTemplate -addControl "multUprLength";
				editorTemplate -addControl "multLwrLength";
				editorTemplate -addSeparator;
				editorTemplate -addControl "stretch";
				editorTemplate -addControl "soft";
				editorTemplate -addControl "slide";
				editorTemplate -addControl "pin"; 
			editorTemplate -endLayout;

			editorTemplate -beginLayout "Input Matrices" -collapse 1;
				editorTemplate -addControl "rootMatrix";
				editorTemplate -addControl "poleMatrix";
				editorTemplate -addControl "goalMatrix";
			editorTemplate -endLayout;

			editorTemplate -beginLayout "Output Values" -collapse 0;
				editorTemplate -addControl "invert";
				editorTemplate -addControl "outUprLength";
				editorTemplate -addControl "outLwrLength";
				editorTemplate -addControl "outIkHandleLocalMatrix";
			editorTemplate -endLayout;

			editorTemplate -addExtraControls;
		editorTemplate -endScrollLayout;
	}
		)";

		MGlobal::executeCommandOnIdle(melCommand, false);
}