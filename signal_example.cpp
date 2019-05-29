// taken (slightly modified) from https://stackoverflow.com/a/20828366
// All credit goes to Tanner Sansbury

#include <thread> // std::thread, std::chrono
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>

/// @brief Mockup class.
class MyClass
{
public:
    /// @brief Connect a slot to the signal.
    template <typename Slot>
    void connect_slot(const Slot& slot)
    {
        signal_.connect(slot);
    }

    /// @brief Send an event to the signal.
    void event(int value)
    {
        signal_(value);
    }

private:
    boost::signals2::signal<void(int)> signal_;
};

/// @brief RAII class used to lock and unlock the GIL.
class gil_lock
{
public:
    gil_lock()  { state_ = PyGILState_Ensure(); }
    ~gil_lock() { PyGILState_Release(state_);   }
private:
    PyGILState_STATE state_;
};

/// @brief Helepr type that will manage the GIL for a python slot.
///
/// @detail GIL management:
///           * Caller must own GIL when constructing py_slot, as 
///             the python::object will be copy-constructed (increment
///             reference to the object)
///           * The newly constructed python::object will be managed
///             by a shared_ptr.  Thus, it may be copied without owning
///             the GIL.  However, a custom deleter will acquire the
///             GIL during deletion.
///           * When py_slot is invoked (operator()), it will acquire
///             the GIL then delegate to the managed python::object.
struct py_slot
{
public:

    /// @brief Constructor that assumes the caller has the GIL locked.
    py_slot(const boost::python::object& object)
            : object_(
            new boost::python::object(object),  // GIL locked, so copy.
            [](boost::python::object* object)   // Delete needs GIL.
            {
                gil_lock lock;
                delete object;
            }
    )
    {}

    // Use default copy-constructor and assignment-operator.
    py_slot(const py_slot&) = default;
    py_slot& operator=(const py_slot&) = default;

    template <typename ...Args>
    void operator()(Args... args)
    {
        // Lock the GIL as the python object is going to be invoked.
        gil_lock lock;
        (*object_)(args...);
    }

private:
    boost::shared_ptr<boost::python::object> object_;
};

/// @brief MyClass::connect_slot helper.
template <typename ...Args>
void MyClass_connect_slot(
        MyClass& self,
        boost::python::object object)
{
    py_slot slot(object); // Adapt object to a py_slot for GIL management.

    // Using a lambda here allows for the args to be expanded automatically.
    // If bind was used, the placeholders would need to be explicitly added.
    self.connect_slot([slot](Args... args) mutable { slot(args...); });
}

/// @brief Sleep then invoke an event on MyClass.
template <typename ...Args>
void MyClass_event_in_thread(
        boost::shared_ptr<MyClass> self,
        unsigned int seconds,
        Args... args)
{
    // Sleep without the GIL.
    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    // We do not want to hold the GIL while invoking or copying
    // C++-specific slots connected to the signal.  Thus, it is the
    // responsibility of python slots to manage the GIL via the
    // py_slot wrapper class.
    self->event(args...);
}

/// @brief Function that will be exposed to python that will create
///        a thread to call the signal.
template <typename ...Args>
void MyClass_event_in(
        boost::shared_ptr<MyClass> self,
        unsigned int seconds,
        Args... args)
{
    // The caller may or may not have the GIL.  Regardless, spawn off a
    // thread that will sleep and then invoke an event on MyClass.  The
    // thread will not be joined so detach from it.  Additionally, as
    // shared_ptr is thread safe, copies of it can be made without the
    // GIL.
    // Note: MyClass_event_in_thread could be expressed as a lambda,
    //       but unpacking a template pack within a lambda does not work
    //       on some compilers.
    std::thread(&MyClass_event_in_thread<Args...>, self, seconds, args...)
            .detach();
}

BOOST_PYTHON_MODULE(signal_example)
{
    PyEval_InitThreads(); // Initialize GIL to support non-python threads.

    namespace python = boost::python;
    python::class_<MyClass, boost::shared_ptr<MyClass>,
            boost::noncopyable>("MyClass")
            .def("connect_slot", &MyClass_connect_slot<int>)
            .def("event",        &MyClass::event)
            .def("event_in",     &MyClass_event_in<int>)
            ;
}