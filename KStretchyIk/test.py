import maya.cmds as cmds

# CREATE RIG
joint1 = cmds.joint(position=[0, 0, 0])
joint2 = cmds.joint(position=[3, 0, -1])
joint3 = cmds.joint(position=[6, 0, 0])
cmds.setAttr(f'{joint1}.overrideEnabled', 1)
cmds.setAttr(f'{joint1}.overrideColor', 13)

cmds.joint(joint1, edit=True, zeroScaleOrient=True, 
           orientJoint='xyz', secondaryAxisOrient='yup')
           
cmds.joint(joint2, edit=True, zeroScaleOrient=True, 
           orientJoint='xyz', secondaryAxisOrient='yup')
           
cmds.joint(joint3, edit=True, zeroScaleOrient=True, 
           orientJoint='xyz', secondaryAxisOrient='yup')

masterGroup = cmds.createNode('transform', n='master')

rootGroup   = cmds.createNode('transform', n='root')
cmds.parent(joint1, rootGroup)
cmds.parent(rootGroup, masterGroup)

endIk = cmds.circle(n='endIk', nr=[1, 0, 0])[0]
cmds.setAttr(f'{endIk}.tx', 6)
cmds.parent(endIk, masterGroup)

poleIk = cmds.circle(n='poleIk', nr=[0, 1, 0])[0]
cmds.setAttr(f'{poleIk}.tx', 3)
cmds.setAttr(f'{poleIk}.tz', -3)
cmds.parent(poleIk, masterGroup)

ikhandle = cmds.ikHandle(n='endIkhandle', sj=joint1, ee=joint3, sol='ikRPsolver')[0]
cmds.setAttr(f'{ikhandle}.v', 0)
cmds.poleVectorConstraint(poleIk, ikhandle)
cmds.parent(ikhandle, endIk)

# CREATE IKSTRETCHNODE
node = cmds.createNode('kStretchyIk')
cmds.setAttr(f'{node}.restUprLength', abs(cmds.getAttr(f'{joint2}.tx')))
cmds.setAttr(f'{node}.restLwrLength', abs(cmds.getAttr(f'{joint3}.tx')))

cmds.connectAttr(f'{endIk}.worldMatrix[0]', f'{node}.goalMatrix')
cmds.connectAttr(f'{poleIk}.worldMatrix[0]', f'{node}.poleMatrix')
cmds.connectAttr(f'{rootGroup}.worldMatrix[0]', f'{node}.rootMatrix')
cmds.connectAttr(f'{node}.outLwrLength', f'{joint3}.tx')
cmds.connectAttr(f'{node}.outUprLength', f'{joint2}.tx')
cmds.connectAttr(f'{node}.outIkHandleLocalMatrix', f'{ikhandle}.offsetParentMatrix')

# CONNECT ATTRS
cmds.addAttr(endIk, ln='_______', at='enum', en='CONTROLS', keyable=False)
cmds.setAttr(f'{endIk}._______', cb=True, lock=True)
cmds.addAttr(endIk, ln='uprLength', proxy=f'{node}.multUprLength')
cmds.addAttr(endIk, ln='lwrLength', proxy=f'{node}.multLwrLength')
cmds.addAttr(endIk, ln='stretch', proxy=f'{node}.stretch')
cmds.addAttr(endIk, ln='soft', proxy=f'{node}.soft')
cmds.addAttr(endIk, ln='slide', proxy=f'{node}.slide')

cmds.addAttr(poleIk, ln='_______', at='enum', en='CONTROLS', keyable=False)
cmds.setAttr(f'{poleIk}._______', cb=True, lock=True)
cmds.addAttr(poleIk, ln='pin', proxy=f'{node}.pin')

cmds.select(endIk)


