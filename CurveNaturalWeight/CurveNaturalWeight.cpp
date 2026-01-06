#include "CurveNaturalWeight.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MFnData.h>
#include <maya/MFnNurbsCurve.H>
#include <maya/MFn.H>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h >


MTypeId CurveNaturalWeight::TYPE_ID(0x00141B83);
MString CurveNaturalWeight::TYPE_NAME("curveNaturalWeight");

MObject CurveNaturalWeight::INPUT_CURVE;
MObject CurveNaturalWeight::WEIGHT;
MObject CurveNaturalWeight::OUTPUT_WEIGHT;

CurveNaturalWeight::CurveNaturalWeight() :MPxNode() {}
CurveNaturalWeight::~CurveNaturalWeight() {}

void* CurveNaturalWeight::creator()
{
	return new CurveNaturalWeight{};
}


MStatus CurveNaturalWeight::initialize()
{
	MFnTypedAttribute   typedAttr{};
	MFnNumericAttribute numericAttr{};

	CurveNaturalWeight::INPUT_CURVE = typedAttr.create("inCurve", "ic", MFnData::kNurbsCurve);
	typedAttr.setKeyable(true);
	typedAttr.setStorable(true);
	typedAttr.setReadable(true);
	typedAttr.setWritable(true);
	typedAttr.setCached(false);

	CurveNaturalWeight::WEIGHT = numericAttr.create("inWeight", "iw", MFnNumericData::kDouble, 0.0);
	numericAttr.setMin(0.0);
	numericAttr.setMax(1.0);
	numericAttr.setStorable(true);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);
	numericAttr.setArray(true);
	numericAttr.setUsesArrayDataBuilder(true);

	CurveNaturalWeight::OUTPUT_WEIGHT = numericAttr.create("outWeight", "ow", MFnNumericData::kDouble, 0.0);
	numericAttr.setStorable(false);
	numericAttr.setArray(true);
	numericAttr.setWritable(true);
	numericAttr.setUsesArrayDataBuilder(true);

	CurveNaturalWeight::addAttribute(CurveNaturalWeight::INPUT_CURVE);
	CurveNaturalWeight::addAttribute(CurveNaturalWeight::WEIGHT);
	CurveNaturalWeight::addAttribute(CurveNaturalWeight::OUTPUT_WEIGHT);
	
	CurveNaturalWeight::attributeAffects(CurveNaturalWeight::INPUT_CURVE, CurveNaturalWeight::OUTPUT_WEIGHT);
	CurveNaturalWeight::attributeAffects(CurveNaturalWeight::WEIGHT, CurveNaturalWeight::OUTPUT_WEIGHT);
	CurveNaturalWeight::setupUI();
	return MS::kSuccess;
}



MStatus CurveNaturalWeight::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	if (plug != CurveNaturalWeight::OUTPUT_WEIGHT) 
		return MS::kUnknownParameter;
	
	MObject curveObj = dataBlock.inputValue(CurveNaturalWeight::INPUT_CURVE).asNurbsCurveTransformed(); // get curve global space

	if (not curveObj.hasFn(MFn::kNurbsCurve))
	{
		dataBlock.setClean(plug);
		return MS::kSuccess;
	}

	MFnNurbsCurve curveFn{curveObj};

	MArrayDataHandle  inWeightH   = dataBlock.inputArrayValue(CurveNaturalWeight::WEIGHT);
	MArrayDataHandle  outWeightH  = dataBlock.outputArrayValue(CurveNaturalWeight::OUTPUT_WEIGHT);
	MArrayDataBuilder outBuileder = outWeightH.builder();

	double curveLength = curveFn.length();

	for (unsigned int index{0}; index < inWeightH.elementCount(); ++index)
	{
		double inWeight		 = inWeightH.inputValue().asDouble();
		double naturalWeight = curveFn.findParamFromLength(inWeight * curveLength);

		outBuileder.addElement(inWeightH.elementIndex()).setDouble(naturalWeight);
		inWeightH.next();
	}
	outWeightH.set(outBuileder);
	outWeightH.setAllClean();

	dataBlock.setClean(plug);
	return MS::kSuccess;
};

void CurveNaturalWeight::setupUI()
{

	const char* melCommand = R"mel(
    global proc AEcurveNaturalWeightTemplate(string $nodeName)
    {
        editorTemplate -beginScrollLayout;

            editorTemplate -beginLayout "Main" -collapse 0;
                editorTemplate -callCustom ("AEinputNew \"In Curve\"") 
                                           ("AEinputReplace \"In Curve\"") 
                                            "inCurve";  
                editorTemplate -addControl "inWeight";
				editorTemplate -addSeparator;
                editorTemplate -addControl "outWeight";
            editorTemplate -endLayout;
    
            AEdependNodeTemplate $nodeName;
            editorTemplate -addExtraControls;
        editorTemplate -endScrollLayout;
    }
    )mel";

	MGlobal::executeCommandOnIdle(melCommand);
}
