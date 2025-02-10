#include "advancedConsole.h"

adv::_constructor::_constructor() {	
	_advancedConsoleConstruct();
}

adv::_constructor::~_constructor() {
	_advancedConsoleDestruct();
}

void adv::error(char* err) {
	_advancedConsoleDestruct();
	printf("Error: %s\r\n", err);
	exit(1);
}

void adv::error(const char* err) {
	adv::error((char*)err);
}

adv::_constructor adv::construct;
std::thread adv::uiloop;
std::mutex adv::buffers;
bool adv::ascii;
bool adv::run;
bool adv::ready;
bool adv::modify;
int adv::width;
int adv::height;
int adv::bufferWidth;
int adv::bufferHeight;
wchar_t* adv::fb;
wchar_t* adv::oldfb;
color_t* adv::cb;
color_t* adv::oldcb;
int adv::drawingMode;
bool adv::disableThreadSafety;
bool adv::doubleSize;
bool adv::thread;
std::condition_variable adv::cvStart;
std::mutex adv::startLock;
std::condition_variable adv::cvThreadState;
std::mutex adv::threadStateMux;
float adv::frametime;
std::chrono::time_point<std::chrono::high_resolution_clock> adv::lastFrame;
