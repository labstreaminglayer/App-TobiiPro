#include "TobiiLib.h"
#include <iostream>
#include <tobii_research.h>
#include <tobii_research_eyetracker.h>

int main(int argc, char **argv) {
	TobiiResearchEyeTrackers *eyetrackers = nullptr;

	auto result = tobii_research_find_all_eyetrackers(&eyetrackers);
	if (result != TOBII_RESEARCH_STATUS_OK) {
		std::cout << "Finding trackers failed. Error: " << result << std::endl;
		return result;
	}
	for (auto i = 0; i < eyetrackers->count; i++) {
		TobiiResearchEyeTracker *eyetracker = eyetrackers->eyetrackers[i];
		std::cout << tobii_str_wrap(&tobii_research_get_address, eyetracker) << '\t'
				  << tobii_str_wrap(&tobii_research_get_serial_number, eyetracker) << '\t'
				  << tobii_str_wrap(&tobii_research_get_device_name, eyetracker) << '\n';
	}
	std::cout << "Found " << (int)eyetrackers->count << " Eye Trackers \n\n" << std::endl;
	tobii_research_free_eyetrackers(eyetrackers);
	return result;
}
