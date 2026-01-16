#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include "KNoFlipSplineSolver.h"


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

    status = pluginFn.registerNode(KNoFlipSplineSolver::TYPE_NAME,
                                   KNoFlipSplineSolver::TYPE_ID,
                                   KNoFlipSplineSolver::creator,
                                   KNoFlipSplineSolver::initialize,
                                   KNoFlipSplineSolver::kDependNode);
    if (not status)
    {
        MGlobal::displayError("Failed to register node: " + KNoFlipSplineSolver::TYPE_NAME);
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject pluginObj)
{
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

    status = pluginFn.deregisterNode(KNoFlipSplineSolver::TYPE_ID);
    if (not status)
    {
        MGlobal::displayError("Failed to deregister node: " + KNoFlipSplineSolver::TYPE_NAME);
        return status;
    }

    return status;

}