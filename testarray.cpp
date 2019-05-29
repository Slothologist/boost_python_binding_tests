
// boost includes for bindings
#include <boost/python.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 106300
    #include <boost/numpy.hpp>
#else
    #include <boost/python/numpy.hpp>
#endif

// std includes
#include <vector>
#include <iostream>

// some namespaces to reduce obfuscation
namespace bp = boost::python;
#if BOOST_VERSION < 106300
    namespace np = boost::numpy;
#else
    namespace np = boost::python::numpy;
#endif


// includes from original start
#include <string>
#include <functional>
#include <sox.h>
// includes from original end



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

	/**
	 * Does not compile somehow.
	void call_function_parameter(boost::function<void(int)> callback){
		callback(100);
	}
	 */ 

	/**
	 * This however does compile
	 */
	void call_function_two_parameter(boost::function<void(int, int)> callback){
		callback(50, 50);
	}

    void call_function_parameter_ndarray(boost::function<void( np::ndarray, int )> callback){
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
        callback(ret_val, 100);
    }

    boost::function<void(std::vector<int>*, int)> callback_cpp;

    void register_callback_func(const boost::function<void( np::ndarray, int )> &callback_py){
        std::cout << "register callback func" << std::endl << std::flush;
        auto lambda = [&](std::vector<int>* vec, int unused){
            std::cout << "callback cpp" << std::endl << std::flush;
            bp::object own;
            np::dtype d_type = np::dtype::get_builtin<int>();
            bp::tuple shape = bp::make_tuple(100);
            bp::tuple stride = bp::make_tuple(sizeof(int));
            std::cout << "stride created" << std::endl << std::flush;

            np::ndarray arr = np::from_data(vec, d_type, shape, stride, own);
            std::cout << "ndarray successfully created" << std::endl << std::flush;

            callback_py(arr, 100);
            std::cout << "callback_py finished" << std::endl << std::flush;
        };
        //callback_cpp = lambda;
    }

    void call_callback_func(){
        std::vector<int>* vec = get_test_array();
        std::cout << "vec created" << std::endl << std::flush;
        callback_cpp(vec, 100);
        std::cout << "callback cpp finished" << std::endl << std::flush;
        delete vec;
    }

    void operator=(ArrayTester const &) = delete;

    };

BOOST_PYTHON_MODULE(testarray){
	np::initialize();
	
	bp::def("create_nd_array", create_nd_array);

	bp::class_<ArrayTester>("ArrayTester")
	.def("create_nd_array_class", &ArrayTester::create_nd_array_class)
	.def("create_nd_array_with_lambda", &ArrayTester::create_nd_array_with_lambda)
	.def("cnda_from_data", &ArrayTester::cnda_from_data)
	.def("call_function", &ArrayTester::call_function)
	//.def("call_function_parameter", &ArrayTester::call_function_parameter)
	.def("call_function_two_parameter", &ArrayTester::call_function_two_parameter)
	;

	function_converter()
        .from_python<void()>()
	//.from_python<void(int)>()
        .from_python<void(int, int)>()
        ;
}
