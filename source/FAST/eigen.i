%{
#define SWIG_FILE_WITH_INIT
#include "Eigen/Dense"
#include <Python.h>
#include <numpy/arrayobject.h>
%}

%init
%{
  import_array();
%}

%include "numpy.i"

%typemap(in) Eigen::Vector3f (Eigen::Vector3f TEMP)
{
  PyArrayObject *temp=NULL;
  if (PyArray_Check($input))
      temp = (PyArrayObject*)$input;

  //TEMP.resize(3);
  TEMP.fill(0);
  double *  values = ((double *) PyArray_DATA(temp));
  for (int i = 0; i < 3; ++i){
          std::cout << "data " << values[i] << std::endl;
          TEMP(i) = (float)values[i];
  }
  $1 = TEMP;

}
