#warning "Using version with stupid ooptions parsing"

#include <exception>
#include <stdexcept>
#include <iostream>

import interpreter;
// import esd;

int main(int argc, char* argv[]) try
{
    interpreter::interpret(argv[1]);
    return 0;
}
catch (std::exception const & e)
{
    std::cerr << "Exception catched: " << e.what() << "\n";
    // esd::show_exception_stacktrace();
    return 1;
}
catch (...)
{
    std::cerr << "Exception catched: undefined.\n";
    // esd::show_exception_stacktrace();
    return 1;
}
