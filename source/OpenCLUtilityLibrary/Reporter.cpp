#include "Reporter.hpp"

#include <iostream>

namespace oul
{

Reporter::Reporter()
{}

Reporter::~Reporter()
{}

void Reporter::report(std::string message, ReporterLevel level){
	std::cout << message << std::endl;
}

} /* namespace oul */
