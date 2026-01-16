from maya import cmds

def demo1():
    multiSampleAttr = 'hardwareRenderingGlobals.multiSampleEnable'
    cmds.setAttr(multiSampleAttr, not cmds.getAttr(multiSampleAttr))

    node = cmds.createNode('kNoFlipSplineSolver')
    
    curve = mel.eval('curve -d 3 -p -11 0 0 -p -8 0 0 -p -5 0 0 -p -2 0 0 -p 1 0 0 -p 4 0 0 -p 7 0 0 -p 10 0 0 -k 0 -k 0 -k 0 -k 1 -k 2 -k 3 -k 4 -k 5 -k 5 -k 5 ;')
    cmds.setAttr(f'{curve}.overrideEnabled', 1)
    cmds.setAttr(f'{curve}.overrideColor', 18)
    cmds.connectAttr(f'{curve}.worldSpace[0]', f'{node}.curve')
        
    for i in range(8):
        locator = cmds.spaceLocator()[0]
        cmds.setAttr(f'{locator}.overrideEnabled', 1)
        cmds.setAttr(f'{locator}.overrideColor', 17)
        cmds.xform(locator, t=cmds.xform(f'{curve}.cv[{i}]', q=True, t=True, ws=True), ws=True)
        cmds.connectAttr(f'{locator}.t', f'{curve}.controlPoints[{i}]')
        cmds.connectAttr(f'{locator}.worldMatrix[0]', f'{node}.handles[{i}].worldMatrix')
    
    joints = []
    jointsCount = 30
    for i in range(jointsCount):
        joint = cmds.createNode('joint')
        joints.append(joint)
        weight = i / (jointsCount - 1)
        cmds.setAttr(f'{node}.offsets[{i}]', weight)
        cmds.setAttr(f'{joint}.displayLocalAxis', 1)
        
    for i, joint in enumerate(joints):
        cmds.connectAttr(f'{node}.outMatrix[{i}]', f'{joint}.offsetParentMatrix')
      
        
def demo2():
    multiSampleAttr = 'hardwareRenderingGlobals.multiSampleEnable'
    cmds.setAttr(multiSampleAttr, not cmds.getAttr(multiSampleAttr))

    node = cmds.createNode('kNoFlipSplineSolver')
    
    curve = mel.eval('curve -d 3 -p -6 0 0 -p -3 0 0 -p 0 0 0 -p 3 0 0 -p 6 0 0 -k 0 -k 0 -k 0 -k 1 -k 2 -k 2 -k 2 ;')
    cmds.setAttr(f'{curve}.overrideEnabled', 1)
    cmds.setAttr(f'{curve}.overrideColor', 18)
    cmds.connectAttr(f'{curve}.worldSpace[0]', f'{node}.curve')
    
    joint1 = cmds.createNode('joint')
    joint2 = cmds.createNode('joint')
    joint3 = cmds.createNode('joint')
    skinJoints = [joint1, joint2, joint3]
    for j in skinJoints:
        cmds.setAttr(f'{j}.radius', 5)
        cmds.setAttr(f'{j}.overrideEnabled', 1)
        cmds.setAttr(f'{j}.overrideColor', 17)
    
    cmds.xform(joint1, t=cmds.xform(f'{curve}.cv[0]', q=True, t=True, ws=True), ws=True)
    cmds.xform(joint2, t=cmds.xform(f'{curve}.cv[2]', q=True, t=True, ws=True), ws=True)
    cmds.xform(joint3, t=cmds.xform(f'{curve}.cv[4]', q=True, t=True, ws=True), ws=True)
    
    cmds.connectAttr(f'{joint1}.worldMatrix[0]', f'{node}.handles[0].worldMatrix')
    cmds.connectAttr(f'{joint2}.worldMatrix[0]', f'{node}.handles[1].worldMatrix')
    cmds.connectAttr(f'{joint3}.worldMatrix[0]', f'{node}.handles[2].worldMatrix')
    
    cmds.skinCluster(skinJoints, curve, tsb=True, bm=1, nw=1)
    
    joints = []
    jointsCount = 10
    for i in range(jointsCount):
        joint = cmds.createNode('joint')
        joints.append(joint)
        weight = i / (jointsCount - 1)
        cmds.setAttr(f'{node}.offsets[{i}]', weight)
        cmds.setAttr(f'{joint}.displayLocalAxis', 1)
        
    for i, joint in enumerate(joints):
        cmds.connectAttr(f'{node}.outMatrix[{i}]', f'{joint}.offsetParentMatrix')
        
        
if __name__ == "__main__":
    
    demo1()
    #demo2()
    