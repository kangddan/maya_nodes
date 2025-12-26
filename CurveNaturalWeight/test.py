from maya import cmds

def main():
    curve = cmds.curve(p=[[0, 0, 0], [2, 0, 0], [4, 0, 0], [6, 0, 0]], d=3)
    node  = cmds.createNode('curveNaturalWeight')

    cmds.connectAttr(f'{curve}.worldSpace', f'{node}.inCurve')

    for i in range(10):
        cmds.setAttr(f'{node}.inWeight[{i}]', i / float(10 - 1))
        node2 = cmds.createNode('pointOnCurveInfo')
        cmds.connectAttr(f'{curve}.worldSpace', f'{node2}.inputCurve')
        cmds.connectAttr(f'{node}.outWeight[{i}]', f'{node2}.parameter')
        
        joint = cmds.createNode('joint')
        cmds.connectAttr(f'{node2}.position', f'{joint}.translate')

    cmds.select(f'{curve}.cv[1]')

if __name__ == '__main__':
    main()
        
