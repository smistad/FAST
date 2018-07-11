namespace fast {

template <class T>
class SharedPointer {
    public:
    	SharedPointer();
		SharedPointer(T* object);
        template <class U>
        SharedPointer(SharedPointer<U> object);
        template <class D>
        SharedPointer(T* p, D d);
        T* get();
        T* operator->();
};

}

// Below are cryptic SWIG macro and typemap definitions copied from boost_shared_ptr.i
// and changed to work for fast::SharedPointer.
// This is needed for casting to work from SharedPointer<Derived> to SharedPointer<Base>

%fragment("SWIG_null_deleter", "header") {
struct SWIG_null_deleter {
  void operator() (void const *) const {
  }
};
%#define SWIG_NO_NULL_DELETER_0 , SWIG_null_deleter()
%#define SWIG_NO_NULL_DELETER_1
%#define SWIG_NO_NULL_DELETER_SWIG_POINTER_NEW
%#define SWIG_NO_NULL_DELETER_SWIG_POINTER_OWN
}


// Workaround empty first macro argument bug
#define SWIGEMPTYHACK
// Main user macro for defining shared_ptr typemaps for both const and non-const pointer types
%define %shared_ptr(TYPE...)
%feature("smartptr", noblock=1) TYPE { fast::SharedPointer< TYPE > }
SWIG_SHARED_PTR_TYPEMAPS(SWIGEMPTYHACK, TYPE)
SWIG_SHARED_PTR_TYPEMAPS(const, TYPE)
%enddef

// Set SHARED_PTR_DISOWN to $disown if required, for example
// #define SHARED_PTR_DISOWN $disown
#if !defined(SHARED_PTR_DISOWN)
#define SHARED_PTR_DISOWN 0
#endif

%fragment("SWIG_null_deleter_python", "header", fragment="SWIG_null_deleter") {
%#define SWIG_NO_NULL_DELETER_SWIG_BUILTIN_INIT
}

// Language specific macro implementing all the customisations for handling the smart pointer
%define SWIG_SHARED_PTR_TYPEMAPS(CONST, TYPE...)

// %naturalvar is as documented for member variables
%naturalvar TYPE;
%naturalvar fast::SharedPointer< CONST TYPE >;

// destructor wrapper customisation
%feature("unref") TYPE 
//"if (debug_shared) { cout << \"deleting use_count: \" << (*smartarg1).use_count() << \" [\" << (boost::get_deleter<SWIG_null_deleter>(*smartarg1) ? std::string(\"CANNOT BE DETERMINED SAFELY\") : ( (*smartarg1).get() ? (*smartarg1)->getValue() : std::string(\"NULL PTR\") )) << \"]\" << endl << flush; }\n"
                               "(void)arg1; delete smartarg1;"

// Typemap customisations...

// plain value
%typemap(in) CONST TYPE (void *argp, int res = 0) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (!argp) {
    %argument_nullref("$type", $symname, $argnum);
  } else {
    $1 = *(%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *)->get());
    if (newmem & SWIG_CAST_NEW_MEMORY) delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
  }
}
%typemap(out) CONST TYPE {
  fast::SharedPointer< CONST TYPE > *smartresult = new fast::SharedPointer< CONST TYPE >(new $1_ltype(($1_ltype &)$1));
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

%typemap(varin) CONST TYPE {
  void *argp = 0;
  int newmem = 0;
  int res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %variable_fail(res, "$type", "$name");
  }
  if (!argp) {
    %variable_nullref("$type", "$name");
  } else {
    $1 = *(%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *)->get());
    if (newmem & SWIG_CAST_NEW_MEMORY) delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
  }
}
%typemap(varout) CONST TYPE {
  fast::SharedPointer< CONST TYPE > *smartresult = new fast::SharedPointer< CONST TYPE >(new $1_ltype(($1_ltype &)$1));
  %set_varoutput(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

// plain pointer
// Note: $disown not implemented by default as it will lead to a memory leak of the shared_ptr instance
%typemap(in) CONST TYPE * (void  *argp = 0, int res = 0, fast::SharedPointer< CONST TYPE > tempshared, fast::SharedPointer< CONST TYPE > *smartarg = 0) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), SHARED_PTR_DISOWN | %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    tempshared = *%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    $1 = %const_cast(tempshared.get(), $1_ltype);
  } else {
    smartarg = %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    $1 = %const_cast((smartarg ? smartarg->get() : 0), $1_ltype);
  }
}

%typemap(out, fragment="SWIG_null_deleter_python") CONST TYPE * {
  fast::SharedPointer< CONST TYPE > *smartresult = $1 ? new fast::SharedPointer< CONST TYPE >($1 SWIG_NO_NULL_DELETER_$owner) : 0;
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), $owner | SWIG_POINTER_OWN));
}

%typemap(varin) CONST TYPE * {
  void *argp = 0;
  int newmem = 0;
  int res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %variable_fail(res, "$type", "$name");
  }
  fast::SharedPointer< CONST TYPE > tempshared;
  fast::SharedPointer< CONST TYPE > *smartarg = 0;
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    tempshared = *%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    $1 = %const_cast(tempshared.get(), $1_ltype);
  } else {
    smartarg = %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    $1 = %const_cast((smartarg ? smartarg->get() : 0), $1_ltype);
  }
}
%typemap(varout, fragment="SWIG_null_deleter_python") CONST TYPE * {
  fast::SharedPointer< CONST TYPE > *smartresult = $1 ? new fast::SharedPointer< CONST TYPE >($1 SWIG_NO_NULL_DELETER_0) : 0;
  %set_varoutput(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

// plain reference
%typemap(in) CONST TYPE & (void  *argp = 0, int res = 0, fast::SharedPointer< CONST TYPE > tempshared) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (!argp) { %argument_nullref("$type", $symname, $argnum); }
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    tempshared = *%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    $1 = %const_cast(tempshared.get(), $1_ltype);
  } else {
    $1 = %const_cast(%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *)->get(), $1_ltype);
  }
}
%typemap(out, fragment="SWIG_null_deleter_python") CONST TYPE & {
  fast::SharedPointer< CONST TYPE > *smartresult = new fast::SharedPointer< CONST TYPE >($1 SWIG_NO_NULL_DELETER_$owner);
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

%typemap(varin) CONST TYPE & {
  void *argp = 0;
  int newmem = 0;
  int res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %variable_fail(res, "$type", "$name");
  }
  fast::SharedPointer< CONST TYPE > tempshared;
  if (!argp) {
    %variable_nullref("$type", "$name");
  }
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    tempshared = *%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    $1 = *%const_cast(tempshared.get(), $1_ltype);
  } else {
    $1 = *%const_cast(%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *)->get(), $1_ltype);
  }
}
%typemap(varout, fragment="SWIG_null_deleter_python") CONST TYPE & {
  fast::SharedPointer< CONST TYPE > *smartresult = new fast::SharedPointer< CONST TYPE >(&$1 SWIG_NO_NULL_DELETER_0);
  %set_varoutput(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

// plain pointer by reference
// Note: $disown not implemented by default as it will lead to a memory leak of the shared_ptr instance
%typemap(in) TYPE *CONST& (void  *argp = 0, int res = 0, $*1_ltype temp = 0, fast::SharedPointer< CONST TYPE > tempshared) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), SHARED_PTR_DISOWN | %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    tempshared = *%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    delete %reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *);
    temp = %const_cast(tempshared.get(), $*1_ltype);
  } else {
    temp = %const_cast(%reinterpret_cast(argp, fast::SharedPointer< CONST TYPE > *)->get(), $*1_ltype);
  }
  $1 = &temp;
}
%typemap(out, fragment="SWIG_null_deleter_python") TYPE *CONST& {
  fast::SharedPointer< CONST TYPE > *smartresult = new fast::SharedPointer< CONST TYPE >(*$1 SWIG_NO_NULL_DELETER_$owner);
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

%typemap(varin) TYPE *CONST& %{
#error "varin typemap not implemented"
%}
%typemap(varout) TYPE *CONST& %{
#error "varout typemap not implemented"
%}

// shared_ptr by value
%typemap(in) fast::SharedPointer< CONST TYPE > (void *argp, int res = 0) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (argp) $1 = *(%reinterpret_cast(argp, $&ltype));
  if (newmem & SWIG_CAST_NEW_MEMORY) delete %reinterpret_cast(argp, $&ltype);
}
%typemap(out) fast::SharedPointer< CONST TYPE > {
  fast::SharedPointer< CONST TYPE > *smartresult = $1 ? new fast::SharedPointer< CONST TYPE >($1) : 0;
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

%typemap(varin) fast::SharedPointer< CONST TYPE > {
  int newmem = 0;
  void *argp = 0;
  int res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %variable_fail(res, "$type", "$name");
  }
  $1 = argp ? *(%reinterpret_cast(argp, $&ltype)) : fast::SharedPointer< TYPE >();
  if (newmem & SWIG_CAST_NEW_MEMORY) delete %reinterpret_cast(argp, $&ltype);
}
%typemap(varout) fast::SharedPointer< CONST TYPE > {
  fast::SharedPointer< CONST TYPE > *smartresult = $1 ? new fast::SharedPointer< CONST TYPE >($1) : 0;
  %set_varoutput(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

// shared_ptr by reference
%typemap(in) fast::SharedPointer< CONST TYPE > & (void *argp, int res = 0, $*1_ltype tempshared) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    if (argp) tempshared = *%reinterpret_cast(argp, $ltype);
    delete %reinterpret_cast(argp, $ltype);
    $1 = &tempshared;
  } else {
    $1 = (argp) ? %reinterpret_cast(argp, $ltype) : &tempshared;
  }
}
%typemap(out) fast::SharedPointer< CONST TYPE > & {
  fast::SharedPointer< CONST TYPE > *smartresult = *$1 ? new fast::SharedPointer< CONST TYPE >(*$1) : 0;
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

%typemap(varin) fast::SharedPointer< CONST TYPE > & %{
#error "varin typemap not implemented"
%}
%typemap(varout) fast::SharedPointer< CONST TYPE > & %{
#error "varout typemap not implemented"
%}

// shared_ptr by pointer
%typemap(in) fast::SharedPointer< CONST TYPE > * (void *argp, int res = 0, $*1_ltype tempshared) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (newmem & SWIG_CAST_NEW_MEMORY) {
    if (argp) tempshared = *%reinterpret_cast(argp, $ltype);
    delete %reinterpret_cast(argp, $ltype);
    $1 = &tempshared;
  } else {
    $1 = (argp) ? %reinterpret_cast(argp, $ltype) : &tempshared;
  }
}
%typemap(out) fast::SharedPointer< CONST TYPE > * {
  fast::SharedPointer< CONST TYPE > *smartresult = $1 && *$1 ? new fast::SharedPointer< CONST TYPE >(*$1) : 0;
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
  if ($owner) delete $1;
}

%typemap(varin) fast::SharedPointer< CONST TYPE > * %{
#error "varin typemap not implemented"
%}
%typemap(varout) fast::SharedPointer< CONST TYPE > * %{
#error "varout typemap not implemented"
%}

// shared_ptr by pointer reference
%typemap(in) fast::SharedPointer< CONST TYPE > *& (void *argp, int res = 0, fast::SharedPointer< CONST TYPE > tempshared, $*1_ltype temp = 0) {
  int newmem = 0;
  res = SWIG_ConvertPtrAndOwn($input, &argp, $descriptor(fast::SharedPointer< TYPE > *), %convertptr_flags, &newmem);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "$type", $symname, $argnum); 
  }
  if (argp) tempshared = *%reinterpret_cast(argp, $*ltype);
  if (newmem & SWIG_CAST_NEW_MEMORY) delete %reinterpret_cast(argp, $*ltype);
  temp = &tempshared;
  $1 = &temp;
}
%typemap(out) fast::SharedPointer< CONST TYPE > *& {
  fast::SharedPointer< CONST TYPE > *smartresult = *$1 && **$1 ? new fast::SharedPointer< CONST TYPE >(**$1) : 0;
  %set_output(SWIG_NewPointerObj(%as_voidptr(smartresult), $descriptor(fast::SharedPointer< TYPE > *), SWIG_POINTER_OWN));
}

%typemap(varin) fast::SharedPointer< CONST TYPE > *& %{
#error "varin typemap not implemented"
%}
%typemap(varout) fast::SharedPointer< CONST TYPE > *& %{
#error "varout typemap not implemented"
%}

// Typecheck typemaps
// Note: SWIG_ConvertPtr with void ** parameter set to 0 instead of using SWIG_ConvertPtrAndOwn, so that the casting 
// function is not called thereby avoiding a possible smart pointer copy constructor call when casting up the inheritance chain.
%typemap(typecheck,precedence=SWIG_TYPECHECK_POINTER,noblock=1) 
                      TYPE CONST,
                      TYPE CONST &,
                      TYPE CONST *,
                      TYPE *CONST&,
                      fast::SharedPointer< CONST TYPE >,
                      fast::SharedPointer< CONST TYPE > &,
                      fast::SharedPointer< CONST TYPE > *,
                      fast::SharedPointer< CONST TYPE > *& {
  int res = SWIG_ConvertPtr($input, 0, $descriptor(fast::SharedPointer< TYPE > *), 0);
  $1 = SWIG_CheckState(res);
}


// various missing typemaps - If ever used (unlikely) ensure compilation error rather than runtime bug
%typemap(in) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
#error "typemaps for $1_type not available"
%}
%typemap(out) CONST TYPE[], CONST TYPE[ANY], CONST TYPE (CLASS::*) %{
#error "typemaps for $1_type not available"
%}


%template() fast::SharedPointer< CONST TYPE >;


%enddef
