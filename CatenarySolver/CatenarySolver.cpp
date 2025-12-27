#include "CatenarySolver.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h >

#include <cmath>     
#include <algorithm> 


MTypeId  CatenarySolver::TYPE_ID{ 0x00141B83 };
MString  CatenarySolver::TYPE_NAME{ "catenarySolver" };
double   CatenarySolver::MAIN_EPS{ 1.0e-5 };
unsigned CatenarySolver::MAX_ITER{ 30 };

MObject CatenarySolver::ITEM_COUNT;
MObject CatenarySolver::LENGTH;
MObject CatenarySolver::INPUT1;
MObject CatenarySolver::INPUT2;

MObject CatenarySolver::OUTPUTS;

CatenarySolver::CatenarySolver() :MPxNode() {}
CatenarySolver::~CatenarySolver() {}

void* CatenarySolver::creator()
{
	return new CatenarySolver();
}

MStatus CatenarySolver::initialize()
{
	MFnNumericAttribute numericAttr;

	CatenarySolver::ITEM_COUNT = numericAttr.create("itemCount", "ic", MFnNumericData::kInt);
	numericAttr.setMin(2);
	numericAttr.setSoftMin(1);
	numericAttr.setSoftMax(100);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setWritable(true);

	CatenarySolver::LENGTH = numericAttr.create("length", "l", MFnNumericData::kDouble, 16.0);
	numericAttr.setMin(1.0);
	numericAttr.setSoftMin(1.0);
	numericAttr.setSoftMax(100.0);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setWritable(true);

	CatenarySolver::INPUT1 = numericAttr.create("input1", "in1", MFnNumericData::k3Double);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setWritable(true);

	CatenarySolver::INPUT2 = numericAttr.create("input2", "in2", MFnNumericData::k3Double);
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setWritable(true);

	CatenarySolver::OUTPUTS = numericAttr.create("outputs", "outs", MFnNumericData::k3Double);
	numericAttr.setArray(true);
	numericAttr.setUsesArrayDataBuilder(true);
	numericAttr.setStorable(false);
	numericAttr.setKeyable(false);
	numericAttr.setWritable(false);
	numericAttr.setReadable(true);

	CatenarySolver::addAttribute(CatenarySolver::INPUT1);
	CatenarySolver::addAttribute(CatenarySolver::INPUT2);
	CatenarySolver::addAttribute(CatenarySolver::ITEM_COUNT);
	CatenarySolver::addAttribute(CatenarySolver::LENGTH);
	CatenarySolver::addAttribute(CatenarySolver::OUTPUTS);

	CatenarySolver::attributeAffects(CatenarySolver::INPUT1,     CatenarySolver::OUTPUTS);
	CatenarySolver::attributeAffects(CatenarySolver::INPUT2,     CatenarySolver::OUTPUTS);
	CatenarySolver::attributeAffects(CatenarySolver::ITEM_COUNT, CatenarySolver::OUTPUTS);
	CatenarySolver::attributeAffects(CatenarySolver::LENGTH,     CatenarySolver::OUTPUTS);

	return MS::kSuccess;
}


MStatus CatenarySolver::compute(const MPlug& plug, MDataBlock& dataBlock)
{
	MPlug outPlug = plug.isChild() ? plug.parent() : plug;
	if (outPlug == CatenarySolver::OUTPUTS)
	{
		const MVector& poseA = dataBlock.inputValue(CatenarySolver::INPUT1).asVector();
		const MVector& poseB = dataBlock.inputValue(CatenarySolver::INPUT2).asVector();

		int itemCount = dataBlock.inputValue(CatenarySolver::ITEM_COUNT).asInt();
		double length = dataBlock.inputValue(CatenarySolver::LENGTH).asDouble();

		MVector directionVec = poseB - poseA;
		double distance		 = directionVec.length();

		MVectorArray poseList;

		if (distance >= length)
		{
			for (unsigned index{0}; index < itemCount; ++index)
			{
				double t = static_cast<double>(index) / (itemCount - 1);
				MVector pos = poseA + directionVec * t;
				poseList.append(pos);
			}
		}
		else
		{
			double _x{ directionVec.x };
			double _z{ directionVec.z };

			double horizontalDist{ std::sqrt(_x * _x + _z * _z) };
			double verticalOffset{ directionVec.y };
			double totalLength{ length };

			if (horizontalDist < CatenarySolver::MAIN_EPS)
			{
				double extraHalfLength = (totalLength - std::abs(verticalOffset)) * 0.5;
				double lowestY = std::min(poseA.y, poseB.y) - extraHalfLength;

				for (unsigned index{ 0 }; index < itemCount; ++index)
				{
					double paramT{ static_cast<double>(index) / (itemCount - 1) };
					double poseY = (paramT <= 0.5)
						? (poseA.y + (lowestY - poseA.y) * (paramT * 2.0))
						: (lowestY + (poseB.y - lowestY) * ((paramT - 0.5) * 2.0));
					poseList.append(MVector(poseA.x, poseY, poseA.z));
				}
			}
			else
			{
				double targetHorizontalLen{ std::sqrt(std::max(0.0, totalLength * totalLength - verticalOffset * verticalOffset)) };
				double minA{ 1.0e-3 };
				double maxA{ std::max(2000.0, ((horizontalDist * horizontalDist) / (8.0 * std::max(minA, totalLength - distance)))) };

				for (unsigned index{ 0 }; index < CatenarySolver::MAX_ITER; ++index)
				{
					double midA = 0.5 * (minA + maxA);
					double currentHorizontalLen = 2.0 * midA * std::sinh(horizontalDist / (2.0 * midA));
					
					if (std::abs(currentHorizontalLen - targetHorizontalLen) < CatenarySolver::MAIN_EPS)
						break;

					if (currentHorizontalLen > targetHorizontalLen)
						minA = midA;
					else
						maxA = midA;
				}

				double curveParamA{ 0.5 * (minA + maxA) };

				double halfSpanSinh{ std::sinh(horizontalDist / (2.0 * curveParamA)) };
				double curveOffsetX{ (horizontalDist * 0.5) - curveParamA * std::asinh(verticalOffset / (2.0 * curveParamA * halfSpanSinh)) };
				double curveOffsetY{ -curveParamA * std::cosh(curveOffsetX / curveParamA) };

				const MVector flatDirection{ MVector(_x, 0.0, _z).normal()};

				for (unsigned index{ 0 }; index < itemCount; ++index)
				{
					double paramT{ static_cast<double>(index) / (itemCount - 1) };
					double currentHorizontalPos = horizontalDist * paramT;
					double heightOffset = curveParamA * std::cosh((currentHorizontalPos - curveOffsetX) / curveParamA) + curveOffsetY;

					poseList.append(MVector(poseA.x + flatDirection.x * currentHorizontalPos, 
											poseA.y + heightOffset, 
											poseA.z + flatDirection.z * currentHorizontalPos));
				}
			}

		}
		MArrayDataHandle  outArrayHandle = dataBlock.outputArrayValue(CatenarySolver::OUTPUTS);
		MArrayDataBuilder arrayBuilder   = outArrayHandle.builder();

		for (unsigned index{ 0 }; index < itemCount; ++index)
		{
			MDataHandle outHandle = arrayBuilder.addElement(index);
			outHandle.set3Double(poseList[index].x, poseList[index].y, poseList[index].z);
		}

		outArrayHandle.set(arrayBuilder);
		outArrayHandle.setAllClean();
		dataBlock.setClean(outPlug);
	}

	return MS::kSuccess;


};
