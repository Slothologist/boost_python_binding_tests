import testarray
import numpy

arr = testarray.create_nd_array()
assert arr.shape[0] == 100

tester = testarray.ArrayTester()

arr = tester.create_nd_array_class()
assert arr.shape[0] == 100

arr = tester.create_nd_array_with_lambda()
assert arr.shape[0] == 100

arr = tester.cnda_from_data()
assert arr.shape[0] == 100

a = None

def function():
    global a
    a = 100

tester.call_function(function)
assert a
a = None

# This does not work.
#def function(parameter):
#    global a
#    a = parameter

#tester.call_function_parameter(function)
#assert a == 100
#a = None

def function(parameter1, parameter2):
    global a
    a = parameter1 + parameter2

tester.call_function_two_parameter(function)
assert a == 100
a = None

exit()
