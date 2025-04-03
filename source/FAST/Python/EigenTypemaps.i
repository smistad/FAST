// Swig typemaps for converting between C++ Eigen vector and matrix
// types and Python sequences
%{
#include <Eigen/Dense>
#include <vector>
#include <sstream>
%}

// Define a few useful functions inside a fragment
%fragment("Eigen_Typemap_Fragments", "header")
%{
    void incorrectSizeMessage(Py_ssize_t got, Py_ssize_t expected) {
        std::stringstream ss;
        ss << "Incorrect sequence size: ";
        ss << got;
        ss << " Expected: ";
        ss << expected;
        PyErr_SetString(PyExc_ValueError, ss.str().c_str());
    }

    template <class T>
    bool PySequenceToEigenVector(PyObject* input, T* output) {
        if(!PySequence_Check(input)) {
            PyErr_SetString(PyExc_TypeError, "Expected a sequence (tuple or list).");
            return false;
        }

        // Check size if fixed
        Py_ssize_t size = PySequence_Size(input);
        if(T::RowsAtCompileTime != Eigen::Dynamic) {
            if(size != output->size()) {
                incorrectSizeMessage(size, output->size());
                return false;
            }
        } else {
            output->resize(size);
        }

        auto data = output->data();
        for(Py_ssize_t i = 0; i < output->size(); ++i) {
            PyObject* item = PySequence_GetItem(input, i);
            if(!PyNumber_Check(item)) {
                PyErr_SetString(PyExc_TypeError, "Sequence elements must be numbers.");
                return false;
            }
            data[i] = (typename T::Scalar)PyFloat_AsDouble(item);
        }

        return true;
    }

    template <class T>
    bool EigenVectorToPyTuple(T input, PyObject** output) {
        *output = PyTuple_New(input.size());
        for(int i = 0; i < input.size(); i++) {
            PyTuple_SetItem(*output, i, PyFloat_FromDouble((double)input(i)));
        }
        return true;
    }
%}

// Create a Swig macro which we can use to repeat the same typemaps
// for several different eigen vector classes
%define %eigen_vector_typemaps(CLASS...)

%typemap(in, fragment="Eigen_Typemap_Fragments") CLASS (CLASS temp) {
   if(!PySequenceToEigenVector<CLASS>($input, &temp))
       SWIG_fail;
   $1 = temp;
}

%typemap(out, fragment="Eigen_Typemap_Fragments") CLASS {
    if(!EigenVectorToPyTuple<CLASS>($1, &$result))
        SWIG_fail;
}

// Needed to support overloaded methods
%typecheck(SWIG_TYPECHECK_POINTER) CLASS, const CLASS, const CLASS& {
    $1 = PySequence_Check($input) ? 1 : 0;
}

// std::vector<CLASS > typemaps
%typemap(in, fragment="Eigen_Typemap_Fragments") std::vector<CLASS > (std::vector<CLASS > temp) {
    if(!PySequence_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "Expected a sequence (tuple or list).");
        SWIG_fail;
    }
    temp.resize(PySequence_Size($input));
    // Convert every item
    for(Py_ssize_t i=0; i != PySequence_Size($input); i++) {
        if(!PySequenceToEigenVector<CLASS>(PySequence_GetItem($input, i), &(temp[i])))
            SWIG_fail;
    }
    $1 = temp;
}

// This std::vector<CLASS> * in typemap used when you try to set a member variable
%typemap(in, fragment="Eigen_Typemap_Fragments") std::vector<CLASS > * (std::vector<CLASS > temp) {
    if(!PySequence_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "Expected a sequence (tuple or list).");
        SWIG_fail;
    }
    temp.resize(PySequence_Size($input));
    // Convert every item
    for(Py_ssize_t i = 0; i != PySequence_Size($input); ++i) {
        if(!PySequenceToEigenVector<CLASS>(PySequence_GetItem($input, i), &(temp[i])))
            SWIG_fail;
    }
    $1 = new std::vector<CLASS >(); // Potential memory leak, need to free using freearg typemap
    *$1 = temp;
}

%typemap(freearg) std::vector<CLASS > * {
    // Needed to fix memory leak above
    delete $1;
}

%typemap(out, fragment="Eigen_Typemap_Fragments") std::vector<CLASS > {
    $result = PyList_New($1.size());
    if(!$result)
        SWIG_fail;
    for(Py_ssize_t i = 0; i != $1.size(); i++) {
        PyObject *out;
        if(!EigenVectorToPyTuple((*(&$1))[i], &out))
            SWIG_fail;
        if(PyList_SetItem($result, i, out) == -1)
            SWIG_fail;
    }
}

// This std::vector<CLASS> * in typemap used when you try to get a member variable
%typemap(out, fragment="Eigen_Typemap_Fragments") std::vector<CLASS >* {
    $result = PyList_New($1->size());
    if(!$result)
        SWIG_fail;
    for(Py_ssize_t i = 0; i != $1->size(); ++i) {
        PyObject *out;
        if(!EigenVectorToPyTuple<CLASS>((*$1)[i], &out))
            SWIG_fail;
        if(PyList_SetItem($result, i, out) == -1)
            SWIG_fail;
    }
}

// Needed to support overloaded methods
%typecheck(SWIG_TYPECHECK_POINTER) std::vector<CLASS >, std::vector<CLASS >* {
    $1 = PySequence_Check($input) && ((PySequence_Size($input) == 0) || PySequence_Check(PySequence_GetItem($input, 0))) ? 1 : 0;
}

// std::vector<std::vector<CLASS>> typemaps

%typemap(in, fragment="Eigen_Typemap_Fragments") std::vector<std::vector<CLASS > > (std::vector<std::vector<CLASS > > temp) {
    if(!PySequence_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "Expected a sequence of sequences (tuple or list).");
        SWIG_fail;
    }
    temp.resize(PySequence_Size($input));
    for(Py_ssize_t i = 0; i != PySequence_Size($input); ++i) {
        PyObject* innerList = PySequence_GetItem($input, i);
        if(!PySequence_Check(innerList)) {
            PyErr_SetString(PyExc_TypeError, "Expected a sequence of sequences (tuple or list).");
            SWIG_fail;
        }
        temp[i].resize(PySequence_Size(innerList));
        for(Py_ssize_t j = 0; j != PySequence_Size(innerList); ++j) {
            if(!PySequenceToEigenVector<CLASS>(PySequence_GetItem(innerList, j), &(temp[i][j])))
                SWIG_fail;
        }
    }
    $1 = temp;
}

%typemap(out, fragment="Eigen_Typemap_Fragments") std::vector<std::vector<CLASS > > {
    std::vector<std::vector<CLASS >>* x = (&$1); // Use pointer to avoid unnecessary copy
    $result = PyList_New(x->size());
    if(!$result)
        SWIG_fail;
    for(Py_ssize_t i = 0; i != x->size(); ++i) {
        PyObject* innerList = PyList_New((*x)[i].size());
        for(Py_ssize_t j = 0; j != (*x)[i].size(); ++j) {
            PyObject *out;
            if(!EigenVectorToPyTuple<CLASS>((*x)[i][j], &out))
                SWIG_fail;
            if(PyList_SetItem(innerList, j, out) == -1)
                SWIG_fail;
        }
        if(PyList_SetItem($result, i, innerList) == -1)
            SWIG_fail;
    }
}

// Needed to support overloaded methods
%typecheck(SWIG_TYPECHECK_POINTER) std::vector<std::vector<CLASS > >, std::vector<std::vector<CLASS > >* {
    // TODO improve this
    $1 = PySequence_Check($input) &&
        ((PySequence_Size($input) == 0) || PySequence_Check(PySequence_GetItem($input, 0)))
        ? 1 : 0;
}

%enddef // End eigen vector typemaps macro
// TODO better signed/unsigned integer handling

// Matrix typemaps
%define %eigen_matrix_typemaps(CLASS...)

%typemap(in, fragment="Eigen_Typemap_Fragments") CLASS {
    if(!PySequence_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "Expected a sequence (tuple or list).");
        SWIG_fail;
    }

    if(PySequence_Size($input) != $1.rows()) {
        incorrectSizeMessage(PySequence_Size($input), $1.rows());
        SWIG_fail;
    }

    for(Py_ssize_t i = 0; i != $1.rows(); ++i) {
        PyObject * innerSequence = PySequence_GetItem($input, i);
        if(!PySequence_Check(innerSequence)) {
            PyErr_SetString(PyExc_TypeError, "Expected a sequence (tuple or list).");
            SWIG_fail;
        }
        if(PySequence_Size(innerSequence) != $1.cols()) {
            incorrectSizeMessage(PySequence_Size(innerSequence), $1.cols());
            SWIG_fail;
        }
        for(Py_ssize_t j = 0; j != $1.cols(); ++j) {
            PyObject* obj = PySequence_GetItem(innerSequence, j);
            if(!PyNumber_Check(obj)) {
                PyErr_SetString(PyExc_TypeError, "Values in sequence must be number");
                SWIG_fail;
            } else {
                $1(i, j) = (typename CLASS::Scalar)PyFloat_AsDouble(obj);
            }
        }
    }
}

%typemap(out) CLASS {
    $result = PyList_New($1.rows());
    for(Py_ssize_t i = 0 ; i != $1.rows(); ++i) {
        PyObject* innerList = PyList_New($1.cols());
        for(Py_ssize_t j = 0 ; j != $1.cols(); ++j) {
            PyList_SetItem(innerList, j, PyFloat_FromDouble((double)$1(i, j)));
        }
        PyList_SetItem($result, i, innerList);
    }
}

%enddef // End eigen matrix typemaps macro
