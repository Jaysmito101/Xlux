#include "Klux.hpp"

int main()
{
    klux::Logger::Init();

    klux::log::Warn("Hello, World! {0}", 1);

    klux::Logger::Shutdown();
}