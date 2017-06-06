/**
 * Examples/GUI/SimpleGUI/main.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "NLMGUI2D.hpp"

using namespace fast;

int main() {

	NLMGUI2D::pointer window = NLMGUI2D::New();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
    
#endif
    window->start();

	return 0;
}
