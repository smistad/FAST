/**
 * Examples/GUI/SimpleGUI/main.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "SimpleGUI.hpp"

using namespace fast;

int main() {

	auto window = SimpleGUI::New();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
	window->start();

	return 0;
}
