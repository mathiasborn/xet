import xet

#t1 = xet.TestClass()
#t2 = xet.TestClass(t1, 2)

d = xet.Document()
d.addInput('test.xet')

print(len(d.tokens))
#for t in d.tokens:
#	print(t)


