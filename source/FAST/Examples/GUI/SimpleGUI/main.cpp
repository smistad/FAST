#include "SimpleGUI.hpp"

using namespace fast;

int main() {

	SimpleGUI::pointer window = SimpleGUI::New();
	// This will automatically close the window after 5 seconds, comment this line to remove it:
    window->setTimeout(5*1000);
	window->start();

	return 0;
}
