#include "TobiiLib.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <tobii_research_eyetracker.h>
#include <tobii_research_streams.h>

using namespace std::chrono_literals;
using Tobii = TobiiResearchEyeTracker;

void sync_callback(TobiiResearchTimeSynchronizationData *ts_data, void *) {
	std::cout << "\nDevice time stamp: " << ts_data->device_time_stamp
			  << "\nSystem request time stamp:  " << ts_data->system_request_time_stamp
			  << "\nSystem response time stamp: " << ts_data->system_response_time_stamp
			  << "\nDifference: "
			  << (ts_data->system_response_time_stamp - ts_data->system_request_time_stamp) << "us"
			  << std::endl;
}
void time_synchronization_data_example(Tobii *et) {
	std::cout << "Subscribing to time synchronization data for eye tracker with serial number "
			  << tobii_str_wrap(&tobii_research_get_serial_number, et) << std::endl;
	tobii_research_subscribe_to_time_synchronization_data(et, sync_callback, nullptr);
	std::this_thread::sleep_for(2s);
	tobii_research_unsubscribe_from_time_synchronization_data(et, sync_callback);
	std::cout << "Unsubscribed from time synchronization data.\n" << std::endl;
}

Tobii *create_eyetracker_example(char *address) {
	Tobii *eyetracker;
	auto status = tobii_research_get_eyetracker(address, &eyetracker);
	std::cout << "Get Eye tracker with status: " << status << std::endl;
	if (status != TOBII_RESEARCH_STATUS_OK)
		throw std::runtime_error("Error: " + std::to_string(status));

	std::cout << "\nAddress: " << address << "\nModel: %s\n"
			  << tobii_str_wrap(&tobii_research_get_device_name, eyetracker)
			  << "\nName (It's OK if this is empty): "
			  << tobii_str_wrap(&tobii_research_get_model, eyetracker) << "\nSerial number: "
			  << tobii_str_wrap(&tobii_research_get_serial_number, eyetracker) << std::endl;
	return eyetracker;
}

int main(int argc, char **argv) {
	if (argc < 2) throw std::runtime_error("Usage: tobiisynctest tet-tcp://eyetracker-address");
	time_synchronization_data_example(create_eyetracker_example(argv[1]));
}
