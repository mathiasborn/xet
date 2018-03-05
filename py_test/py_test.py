import xet

#t1 = xet.TestClass()
#t2 = xet.TestClass(t1, 2)

#f = xet.PolygonSet([(0,0), (10,0), (100,200)])
#f ^= xet.PolygonSet([(0,0), (10,0), (100,200)])
#f |= ((10,10), (20,20))
#print(f)


d = xet.Document()
d.addInput('test.xet')

print(len(d.tokens))
#for t in d.tokens:
#	print(t)


