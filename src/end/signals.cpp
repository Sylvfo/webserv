#include "Webserv.hpp"

volatile sig_atomic_t WebServ::g_shutdown = 0;

void handleSignInt(int sign)
{
	(void) sign;
	WebServ::setShutdown(true);
}

void WebServ::setShutdown(bool value) {
    g_shutdown = value ? 1 : 0;
}

bool WebServ::shouldShutdown() {
    return g_shutdown != 0;
}