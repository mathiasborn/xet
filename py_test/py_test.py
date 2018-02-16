import xet

d = xet.Document()
d.addInput('test.xet')

print(len(d.tokens))
for t in d.tokens:
	print(t)


