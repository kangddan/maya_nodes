#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include "kChainRoll.h"


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

    status = pluginFn.registerNode(KChainRoll::TYPE_NAME,
        KChainRoll::TYPE_ID,
        KChainRoll::creator,
        KChainRoll::initialize,
        KChainRoll::kDependNode);
    if (not status)
    {
        MGlobal::displayError("Failed to register node: " + KChainRoll::TYPE_NAME);
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject pluginObj)
{
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

    status = pluginFn.deregisterNode(KChainRoll::TYPE_ID);
    if (not status)
    {
        MGlobal::displayError("Failed to deregister node: " + KChainRoll::TYPE_NAME);
        return status;
    }

    return status;

}