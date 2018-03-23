import xet

#t1 = xet.TestClass()
#t2 = xet.TestClass(t1, 2)

#f = xet.PolygonSet([(0,0), (10,0), (100,200)])
#f ^= xet.PolygonSet([(0,0), (10,0), (100,200)])
#f |= ((10,10), (20,20))
f = xet.Rectangle(top=0, left=0, width=10, height=20)
#print(f)


d = xet.Document()
d.addInput('test.xet')

print(len(d.tokens))
for t in d.tokens:
	print(t)

f = d.tokens[0]
ip = f(d)
print(ip)
