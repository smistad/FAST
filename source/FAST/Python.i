%module fast
%{
#include "FAST/ProcessObject.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
%}

%include "FAST/ProcessObject.i"
%include "FAST/Importers/ImageFileImporter.i"
%include "FAST/Visualization/ImageRenderer/ImageRenderer.i"
%include "FAST/Visualization/SimpleWindow.i"