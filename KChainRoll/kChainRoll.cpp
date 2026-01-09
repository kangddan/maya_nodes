#include "KChainRoll.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MRampAttribute.h>
#include <maya/MAngle.h >

#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h >

#include <algorithm>
#include <cmath>
#include <limits>

const double PI = std::acos(-1.0);

MTypeId KChainRoll::TYPE_ID  { 0x00141B93 };
MString KChainRoll::TYPE_NAME{ "kChainRoll" };

// INPUTS
MObject KChainRoll::COUNT;
MObject KChainRoll::ROLL;
MObject KChainRoll::ANGLE;
MObject KChainRoll::FALLOFF;
MObject KChainRoll::CURVE_DATA;

// OUTPUTS
MObject KChainRoll::OUT_ANGLES;

// RAMP DATA
const float defaultPositions[1]{ 1.0f };
const float defaultValues[1]{ 1.0f };
const int   defaultInterps[1]{ MRampAttribute::kLinear };

MFloatArray KChainRoll::POSITIONS { defaultPositions, 1 };
MFloatArray KChainRoll::VALUES    { defaultValues, 1 };
MIntArray   KChainRoll::INTERPS   { defaultInterps, 1 };

KChainRoll::KChainRoll() :MPxNode() {}
KChainRoll::~KChainRoll() {}

void* KChainRoll::creator() { return new KChainRoll{}; }

double KChainRoll::clamp(double v, double _min, double _max)
{
	return std::max(_min, std::min(v, _max));
}

double KChainRoll::remap(double v, double inMin, double inMax)
{
	if (std::abs(inMax - inMin) <= std::numeric_limits<double>::epsilon())
		return 0.0;

	double t = (v - inMin) / (inMax - inMin);
	return KChainRoll::clamp(t, 0.0, 1.0);
}

MStatus KChainRoll::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	if (plug != KChainRoll::OUT_ANGLES)
		return MS::kUnknownParameter;

	int   count    = dataBlock.inputValue(KChainRoll::COUNT).asInt();
	double roll    = dataBlock.inputValue(KChainRoll::ROLL).asDouble();
	double angle   = dataBlock.inputValue(KChainRoll::ANGLE).asAngle().asRadians();
	double folloff = dataBlock.inputValue(KChainRoll::FALLOFF).asDouble();

	MRampAttribute    ramapHandle{ this->thisMObject(), KChainRoll::CURVE_DATA };
	MArrayDataHandle  outArrayHandle  = dataBlock.outputArrayValue(KChainRoll::OUT_ANGLES);
	MArrayDataBuilder outArrayBuilder = outArrayHandle.builder();

	float curveValue { 0.0f };
	int   numSegs    { count - 1 };

	for (unsigned int index{ 0 }; index < count; ++index)
	{
		float weight = static_cast<float>(index) / numSegs;
		ramapHandle.getValueAtPosition(weight, curveValue);

		double weightMax = 1.0 - static_cast<double>(index)	    / count;
		double weightMin = 1.0 - static_cast<double>(index + 1) / count;

		double rollWeight = KChainRoll::remap(roll, KChainRoll::clamp(weightMin - folloff, 0.0, 1.0), weightMax);

		MDataHandle outHandle = outArrayBuilder.addElement(index);
		outHandle.setMAngle(MAngle(angle * rollWeight * curveValue, MAngle::kRadians));
	}

	outArrayHandle.set(outArrayBuilder);
	outArrayHandle.setAllClean();
	dataBlock.setClean(plug);

	return MS::kSuccess;
};

MStatus KChainRoll::initialize()
{
	MFnUnitAttribute    unitAttr{};
	MFnNumericAttribute numericAttr{};

	KChainRoll::COUNT = numericAttr.create("count", "c", MFnNumericData::kInt, 2);
	numericAttr.setMin(2);
	numericAttr.setSoftMax(50);
	numericAttr.setKeyable(false);
	numericAttr.setReadable(false);
	numericAttr.setStorable(true);

	KChainRoll::ROLL = numericAttr.create("roll", "r", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KChainRoll::ANGLE = unitAttr.create("angle", "a", MAngle(PI/3, MAngle::kRadians));
	unitAttr.setSoftMin(MAngle(-180, MAngle::kDegrees));
	unitAttr.setSoftMax(MAngle(180,  MAngle::kDegrees));
	unitAttr.setKeyable(true);
	unitAttr.setReadable(true);
	unitAttr.setStorable(true);

	KChainRoll::FALLOFF = numericAttr.create("falloff", "f", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(true);
	numericAttr.setStorable(true);

	KChainRoll::CURVE_DATA = MRampAttribute::createCurveRamp("curveData", "cd");

	KChainRoll::OUT_ANGLES = unitAttr.create("outputAngles", "oa", MAngle(0, MAngle::kRadians));
	unitAttr.setArray(true);
	unitAttr.setUsesArrayDataBuilder(true);
	unitAttr.setWritable(false);

	KChainRoll::addAttribute(KChainRoll::COUNT);
	KChainRoll::addAttribute(KChainRoll::ROLL);
	KChainRoll::addAttribute(KChainRoll::ANGLE);
	KChainRoll::addAttribute(KChainRoll::FALLOFF);
	KChainRoll::addAttribute(KChainRoll::CURVE_DATA);
	KChainRoll::addAttribute(KChainRoll::OUT_ANGLES);

	KChainRoll::attributeAffects(KChainRoll::COUNT,      KChainRoll::OUT_ANGLES);
	KChainRoll::attributeAffects(KChainRoll::ROLL,       KChainRoll::OUT_ANGLES);
	KChainRoll::attributeAffects(KChainRoll::ANGLE,      KChainRoll::OUT_ANGLES);
	KChainRoll::attributeAffects(KChainRoll::FALLOFF,    KChainRoll::OUT_ANGLES);
	KChainRoll::attributeAffects(KChainRoll::CURVE_DATA, KChainRoll::OUT_ANGLES);
	KChainRoll::attributeAffects(KChainRoll::COUNT,      KChainRoll::OUT_ANGLES);

	KChainRoll::setupUI();
	return MS::kSuccess;
}

void KChainRoll::postConstructor()
{
	MPxNode::postConstructor();
	MRampAttribute splineAttr{ this->thisMObject(), KChainRoll::CURVE_DATA };
	splineAttr.addEntries(KChainRoll::POSITIONS, KChainRoll::VALUES, KChainRoll::INTERPS);
}

void KChainRoll::setupUI()
{

	const char* melCommand = R"mel(
    global proc AEkChainRollTemplate(string $nodeName)
    {
        editorTemplate -beginScrollLayout;

            editorTemplate -beginLayout "Main" -collapse 0;
                editorTemplate -addControl "count";
                editorTemplate -addControl "roll";
                editorTemplate -addControl "angle";
                editorTemplate -addControl "falloff";

				AEaddRampControl( $nodeName + ".curveData");      
                editorTemplate -addControl "outputAngles";
            editorTemplate -endLayout;
    
            AEdependNodeTemplate $nodeName;
            editorTemplate -addExtraControls;
        editorTemplate -endScrollLayout;
    }
    )mel";

	MGlobal::executeCommandOnIdle(melCommand);
}
