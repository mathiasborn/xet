#!/usr/bin/env python
# -*- coding: utf-8 -*-

import xet

class Child(xet.TestClass):
	def __init__(self, **kwds):
		super().__init__(**kwds)


def run():
	print('xet_test')
	t = Child(width=2000*xet.um, adjustment=(23, 24))
	print(t)
