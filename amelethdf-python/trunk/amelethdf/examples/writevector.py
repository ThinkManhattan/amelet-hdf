'''
Created on 18 sept. 2009

@author: Cyril Giraudon
'''

import unittest

import tables
tables.parameters.PYTABLES_SYS_ATTRS = False

import numpy

from amelethdf.floatingtype.vector import Vector

from amelethdf.tables.hdfprint import print_node, open_file

class Test(unittest.TestCase):
    def setUp(self):
        self.h5file = open_file("vector.h5", mode = "a", entry_point = "")

    def tearDown(self):
        self.h5file.close()

    def test_print_int_dataset(self):
        simple = Vector(name = "int_vector")
        simple.label = "an integer vector"
        simple.physicalNature = "length"
        simple.unit = "meter"
        simple.values = numpy.array([1, 2, 3, 4, 5, 6], dtype=numpy.int32)
        print_node(self.h5file, simple, self.h5file.root)

    def test_print_float_dataset(self):
        simple = Vector(name = "float_vector")
        simple.label = "an float vector"
        simple.physicalNature = "length"
        simple.unit = "meter"
        simple.values = numpy.array([1., 2., 3., 4., 5., 6.], dtype=numpy.float32)
        print_node(self.h5file, simple, self.h5file.root)

    def test_print_complex_dataset(self):
        simple = Vector(name = "comp_vector")
        simple.label = "a complex vector"
        simple.physicalNature = "length"
        simple.unit = "meter"
        simple.values = numpy.array([1+0.5j, 2+1.5j, 3+2.5j,\
                                     4+3.5j, 5+4.5j, 6+5.5j], 
                                    dtype=numpy.complex64)
        print_node(self.h5file, simple, self.h5file.root)

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()
