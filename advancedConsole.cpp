#include "advancedConsole.h"

adv::_constructor::_constructor() {	
	_advancedConsoleConstruct();
}

adv::_constructor::~_constructor() {
	_advancedConsoleDestruct();
}

void adv::error(char* err) {
	_advancedConsoleDestruct();
	while (console::ready) 
		console::sleep(50); //wait for destruct
	printf("Error: %s\r\n", err);
	exit(1);
}

adv::_constructor adv::construct;
std::thread adv::uiloop;
std::mutex adv::buffers;
bool adv::run;
bool adv::ready;
bool adv::modify;
int adv::width;
int adv::height;
wchar_t* adv::fb;
wchar_t* adv::oldfb;
char* adv::cb;
char* adv::oldcb;
int adv::drawingMode;
bool adv::doubleSize;
bool adv::thread;
std::condition_variable adv::cvStart;
std::mutex adv::startLock;