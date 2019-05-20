import testarray

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

exit()
