
// boost includes for bindings
#include <boost/python.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 106300
    #include <boost/numpy.hpp>
#else
    #include <boost/python/numpy.hpp>
#endif


// some namespaces to reduce obfuscation
namespace bp = boost::python;
#if BOOST_VERSION < 106300
    namespace np = boost::numpy;
#else
    namespace np = boost::python::numpy;
#endif

void create_nd_array(){
	np::dtype d_type = np::dtype::get_builtin<int>();
	bp::tuple shape = bp::make_tuple(100);
	np::ndarray arr = np::zeros(shape, d_type);
}

BOOST_PYTHON_MODULE(testarray){
	np::initialize();
	
	bp::def("create_nd_array", create_nd_array);
}
