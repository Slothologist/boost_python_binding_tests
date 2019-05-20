
// boost includes for bindings
#include <boost/python.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 106300
    #include <boost/numpy.hpp>
#else
    #include <boost/python/numpy.hpp>
#endif

#include <vector>


// some namespaces to reduce obfuscation
namespace bp = boost::python;
#if BOOST_VERSION < 106300
    namespace np = boost::numpy;
#else
    namespace np = boost::python::numpy;
#endif


struct function_converter {
/// @note Registers converter from a python callable type to the
///       provided type. Taken from https://stackoverflow.com/a/16109416/8155003
template<typename FunctionSig>
function_converter &
from_python() {
    boost::python::converter::registry::push_back(
            &function_converter::convertible,
            &function_converter::construct<FunctionSig>,
            boost::python::type_id < boost::function < FunctionSig >> ());

    // Support chaining.
    return *this;
}

/// @brief Check if PyObject is callable.
static void *convertible(PyObject *object) {
    return PyCallable_Check(object) ? object : NULL;
}

/// @brief Convert callable PyObject to a C++ boost::function.
template<typename FunctionSig>
static void construct(
        PyObject *object,
        boost::python::converter::rvalue_from_python_stage1_data *data) {
    namespace python = boost::python;
    // Object is a borrowed reference, so create a handle indicting it is
    // borrowed for proper reference counting.
    python::handle<> handle(python::borrowed(object));

    // Obtain a handle to the memory block that the converter has allocated
    // for the C++ type.
    typedef boost::function <FunctionSig> functor_type;
    typedef python::converter::rvalue_from_python_storage <functor_type>
            storage_type;
    void *storage = reinterpret_cast<storage_type *>(data)->storage.bytes;

    // Allocate the C++ type into the converter's memory block, and assign
    // its handle to the converter's convertible variable.
    new(storage) functor_type(python::object(handle));
    data->convertible = storage;
}
};

np::ndarray create_nd_array(){
	np::dtype d_type = np::dtype::get_builtin<int>();
	bp::tuple shape = bp::make_tuple(100);
	np::ndarray arr = np::zeros(shape, d_type);
	return arr;
}

std::vector<int>* get_test_array(){
	std::vector<int> * vec = new std::vector<int>();
	for(int i = 0; i < 100; i++){
		vec->push_back(i);
	}
	return vec;
}


class ArrayTester{
public:
	np::ndarray create_nd_array_class(){
		np::dtype d_type = np::dtype::get_builtin<int>();
		bp::tuple shape = bp::make_tuple(100);
		np::ndarray arr = np::zeros(shape, d_type);
		return arr;
	}

	np::ndarray create_nd_array_with_lambda(){
		auto lambda = [&](){
			np::dtype d_type = np::dtype::get_builtin<int>();
			bp::tuple shape = bp::make_tuple(100);
			np::ndarray arr = np::zeros(shape, d_type);
			return arr;
		};
		np::ndarray ret_val = lambda();
		return ret_val;
	} 

	np::ndarray cnda_from_data(){
		std::vector<int>* vec = get_test_array();
		bp::object own;
		auto lambda = [&](){
			np::dtype d_type = np::dtype::get_builtin<int>();
			bp::tuple shape = bp::make_tuple(100);
			bp::tuple stride = bp::make_tuple(sizeof(int));
			np::ndarray arr = np::from_data(vec, d_type, shape, stride, own);
			return arr;
		};
		np::ndarray ret_val = lambda();
		delete vec;
		return ret_val;
	}

	void call_function(boost::function<void()> callback){
		callback();
	}

};

BOOST_PYTHON_MODULE(testarray){
	np::initialize();
	
	bp::def("create_nd_array", create_nd_array);

	bp::class_<ArrayTester>("ArrayTester")
	.def("create_nd_array_class", &ArrayTester::create_nd_array_class)
	.def("create_nd_array_with_lambda", &ArrayTester::create_nd_array_with_lambda)
	.def("cnda_from_data", &ArrayTester::cnda_from_data)
	.def("call_function", &ArrayTester::call_function)
	;

	function_converter()
        .from_python<void()>()
        ;
}
