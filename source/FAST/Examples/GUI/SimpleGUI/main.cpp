/**
 * @example surfaceExtractionGUIExample.cpp
 * An example of a GUI for performing surface extraction from a volume in real-time.
 *
 * main.cpp:
 * @include GUI/SimpleGUI/main.cpp
 * SimpleGUI.hpp:
 * @include GUI/SimpleGUI/SimpleGUI.hpp
 * SimpleGUI.cpp:
 * @include GUI/SimpleGUI/SimpleGUI.cpp
 */
#include "SimpleGUI.hpp"

using namespace fast;

int main() {

	auto window = SimpleGUI::create();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
	window->run();

	return 0;
}
