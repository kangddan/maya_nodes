#pragma once

#include <maya/MPxNode.h>

class MathNode : public MPxNode
{

private:
	static MObject INPUT1;
	static MObject INPUT2;
	static MObject OUTPUT;
	static MObject FUNCTION;

public:
	static MTypeId TYPE_ID;
	static MString TYPE_NAME;

public:
	MathNode();
	virtual ~MathNode() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock) override;
	static void* creator();
	static MStatus initialize();
	

};