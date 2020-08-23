%define %unique_ptr(TYPE)
%typemap (out) std::unique_ptr<TYPE > %{
   %set_output(SWIG_NewPointerObj($1.release(), $descriptor(TYPE *), SWIG_POINTER_OWN | %newpointer_flags));
%}
%template() std::unique_ptr<TYPE >;
%enddef

namespace std {
   template <class T> class unique_ptr {};
}
