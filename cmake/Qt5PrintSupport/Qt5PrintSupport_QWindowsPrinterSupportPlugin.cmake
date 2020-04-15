
add_library(Qt5::QWindowsPrinterSupportPlugin MODULE IMPORTED)


_populate_PrintSupport_plugin_properties(QWindowsPrinterSupportPlugin RELEASE "printsupport/windowsprintersupport.dll" FALSE)

list(APPEND Qt5PrintSupport_PLUGINS Qt5::QWindowsPrinterSupportPlugin)
set_property(TARGET Qt5::PrintSupport APPEND PROPERTY QT_ALL_PLUGINS_printsupport Qt5::QWindowsPrinterSupportPlugin)
set_property(TARGET Qt5::QWindowsPrinterSupportPlugin PROPERTY QT_PLUGIN_TYPE "printsupport")
set_property(TARGET Qt5::QWindowsPrinterSupportPlugin PROPERTY QT_PLUGIN_EXTENDS "")
