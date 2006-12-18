//File : python_grass6.i


%module python_grass6
%{
#include "include/gis.h"
#include "include/gisdefs.h"
%include "cpointer.i"
%pointer_functions(int, intp);
%pointer_functions(float, floatp);
%pointer_functions(double, doublep);

#include "include/imagery.h"
#include "include/imagedefs.h"
%}

%include "my_typemaps.i"
%include "renames.i"
%include "interfaces/gis.i"
%include "interfaces/gisdefs.i"
%include "interfaces/imagery.i"
%include "interfaces/imagedefs.i"


