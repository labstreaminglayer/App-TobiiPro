#ifndef TOBIILIB_H_
#define TOBIILIB_H_

#include <string>
#include <tobii_research_eyetracker.h>
#include <stdexcept>

template<typename Args>
std::string tobii_str_wrap(TobiiResearchStatus (*funcptr)(Args, char**), Args args) {
	char* tmp;
	if(auto res = funcptr(args, &tmp) != TOBII_RESEARCH_STATUS_OK)
		throw std::runtime_error("Error in Tobii Call: " + std::to_string(res));
	std::string res(tmp);
	tobii_research_free_string(tmp);
	return res;
}

#endif
