Plotting module {#plotting-module}
=====================================

The plotting module uses the library [JKQTPlotter](https://github.com/jkriege2/JKQtPlotter) which is based on Qt 5. The reason for using this library instead of the official QtCharts is mainly because QtCharts is GPL licensed, while JKQTPlotter is LGPL licensed.

This module enables you to do real-time plotting by connecting a plotter process object to a FAST pipeline. The plotting process object is updated continuously as it gets new data from its input connections.

You may also use JKQTPlotter directly without plugging it in a FAST pipeline, see examples of how to create great looking plots here: https://github.com/jkriege2/JKQtPlotter/tree/master/examples