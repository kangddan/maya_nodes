#include <cmath>
#include "MathNode.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>

MTypeId MathNode::TYPE_ID{ 0x00080033 };
MString MathNode::TYPE_NAME{ "mathNode" };

MObject MathNode::INPUT1;
MObject MathNode::INPUT2;
MObject MathNode::OUTPUT;
MObject MathNode::FUNCTION;


MathNode::MathNode()
{
}

MathNode::~MathNode()
{
}


MStatus MathNode::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	if (plug == MathNode::OUTPUT)
	{
		double input1 = dataBlock.inputValue(MathNode::INPUT1).asDouble();
		double input2 = dataBlock.inputValue(MathNode::INPUT2).asDouble();
		double result = 0.0;
		int mode = dataBlock.inputValue(MathNode::FUNCTION).asInt();
		switch (mode)
		{
		case 0: // Add
			result = input1 +  input2;
			break;
		case 1: // Subtract
			result = input1 - input2;
			break;
		case 2: // Multiply
			result = input1 * input2;
			break;
		case 3: // Divide
			if (input2 != 0.0)
				result = input1 / input2;
			else
				MGlobal::displayError("Division by zero");
			break;
		case 4: // Modulo
			if (input2 != 0.0)
				result = std::fmod(input1, input2);
			else
				MGlobal::displayError("Modulo by zero");
			break;
		default:
			MGlobal::displayError("Unknown function");
			break;
		}
		dataBlock.outputValue(MathNode::OUTPUT).setDouble(result);
		dataBlock.setClean(plug);
		
	}

	return MS::kSuccess;
}

void* MathNode::creator()
{
	return new MathNode();
}

MStatus MathNode::initialize()
{
	MFnNumericAttribute numericAttr;
	MFnEnumAttribute    enumAttr;

	MathNode::INPUT1 = numericAttr.create("input1", "in1", MFnNumericData::kDouble, 0.0);
	numericAttr.setReadable(false);
	numericAttr.setKeyable(true);

	MathNode::INPUT2 = numericAttr.create("input2", "in2", MFnNumericData::kDouble, 0.0);
	numericAttr.setReadable(false);
	numericAttr.setKeyable(true);

	MathNode::FUNCTION = enumAttr.create("function", "func", 0);
	enumAttr.addField("Add",      0);
	enumAttr.addField("Subtract", 1);
	enumAttr.addField("Multiply", 2);
	enumAttr.addField("Divide",   3);
	enumAttr.addField("Modulo",   4);
	enumAttr.setReadable(true);
	enumAttr.setKeyable(true);

	MathNode::OUTPUT = numericAttr.create("output", "out", MFnNumericData::kDouble, 0.0);
	numericAttr.setStorable(false);
	numericAttr.setWritable(false);

	MathNode::addAttribute(MathNode::INPUT1);
	MathNode::addAttribute(MathNode::INPUT2);
	MathNode::addAttribute(MathNode::OUTPUT);
	MathNode::addAttribute(MathNode::FUNCTION);

	MathNode::attributeAffects(MathNode::INPUT1,   MathNode::OUTPUT);
	MathNode::attributeAffects(MathNode::INPUT2,   MathNode::OUTPUT);
	MathNode::attributeAffects(MathNode::FUNCTION, MathNode::OUTPUT);
	return MS::kSuccess;
}