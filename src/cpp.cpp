#include <hpp.hpp>

template<typename T>
void private_cpp_print(T x) {
    std::cout << "Yourlib.cpp_print(" << x << ")" << std::endl;
}

extern "C" void yourlib_call_cpp(void) {
    private_cpp_print<int>(100);
}
