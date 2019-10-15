print(23)
import xet

#t1 = xet.TestClass()
#t2 = xet.TestClass(t1, 2)

#f = xet.PolygonSet([(0,0), (10,0), (100,200)])
#f ^= xet.PolygonSet([(0,0), (10,0), (100,200)])
#f |= ((10,10), (20,20))
f = xet.Rectangle(top=0, left=0, width=10, height=20)
print(f)

class RectShape(xet.VariablePolygonShapes):
    def __init__(self):
        return super().__init__(0)
    def polygons(self, scale):
        return [xet.Rectangle(top=0, left=0, width=10, height=20*scale)]

rs = RectShape()
print(rs)

d = xet.Document()
d.addInput('test.xet')
d.toPDF('test.pdf')
