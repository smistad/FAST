/**
 * Examples/GUI/SimpleGUI/main.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "NLMGUI3D.hpp"

using namespace fast;

int main() {

	// THIS DOES NOT WORK ATM
	NLMGUI3D::pointer window = NLMGUI3D::New();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
    
#endif
    window->setWidth(1600);
    window->setHeight(600);
    window->start();

	return 0;
}
