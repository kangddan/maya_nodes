#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include "MathNode.h"


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

    status = pluginFn.registerNode(MathNode::TYPE_NAME,
                                   MathNode::TYPE_ID,
                                   MathNode::creator,
                                   MathNode::initialize,
                                   MathNode::kDependNode);
    if (not status)
    {
        MGlobal::displayError("Failed to register node: " + MathNode::TYPE_NAME);
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject pluginObj)
{
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

    status = pluginFn.deregisterNode(MathNode::TYPE_ID);
    if (not status)
    {
        MGlobal::displayError("Failed to deregister node: " + MathNode::TYPE_NAME);
        return status;
    }

    return(status);
}
