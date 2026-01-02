#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include "Matrix2Vectors.h"


MStatus initializePlugin(MObject pluginObj)
{
    const char* vendor = "LinkRigger";
    const char* version = "1.0.0";
    const char* requiredApiVersion = "Any";

    MStatus status;

    MFnPlugin pluginFn(pluginObj, vendor, version, requiredApiVersion, &status);
    if (not status)
    {
        MGlobal::displayError("Failed to initialize plugin: " + status.errorString());
        return status;
    }

    status = pluginFn.registerNode(Matrix2Vectors::TYPE_NAME,
        Matrix2Vectors::TYPE_ID,
        Matrix2Vectors::creator,
        Matrix2Vectors::initialize,
        Matrix2Vectors::kDependNode);
    if (not status)
    {
        MGlobal::displayError("Failed to register node: " + Matrix2Vectors::TYPE_NAME);
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject pluginObj)
{
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

    status = pluginFn.deregisterNode(Matrix2Vectors::TYPE_ID);
    if (not status)
    {
        MGlobal::displayError("Failed to deregister node: " + Matrix2Vectors::TYPE_NAME);
        return status;
    }

    return status;

}

