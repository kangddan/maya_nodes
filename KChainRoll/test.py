from maya import cmds

def main():
    count = 50
    joints = []
    for i in range(count):
        joint = cmds.joint()
        cmds.setAttr(f'{joint}.tx', 1)
        joints.append(joint)
    node = cmds.createNode('kChainRoll')
    cmds.setAttr(f'{node}.count', count)
    for index, joint in enumerate(joints):
        cmds.connectAttr(f'{node}.outputAngles[{index}]', f'{joint}.rz')
    
if __name__ == '__main__':
    main()
