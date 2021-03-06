/*
 * PerfTimer.cpp:
 *  Provides timing information on a per-method basis.
 *  Public domain (by Adam Domurad)
 */

#include <map>
#include <string>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <vector>
#include "PerfTimer.h"

void MethodPerfProfile::begin_timer() {
	timer.start();
}

void MethodPerfProfile::end_timer() {
	long timemicro = timer.get_microseconds();
	total_calls++;
	total_microseconds += timemicro;

	// Note, converting to float can be pricey
	double ftime = timemicro;
	double avg2 = total_microseconds / double(total_calls);

	max_microseconds = std::max(max_microseconds, timemicro);
	qvalue += (ftime - avg) * (ftime - avg2); // Part of a standard deviation formula, hell if I understand it
	avg = avg2;
}

void PerfTimer::begin(const char* method) {
	perf_map[method].begin_timer();
}

void PerfTimer::end(const char* method) {
	perf_map[method].end_timer();
}

struct PProfile {
	std::string func_name;
	MethodPerfProfile profile;
	bool operator<(const PProfile& pp) const {
		return profile.total_microseconds > pp.profile.total_microseconds;
	}
};

void PerfTimer::print_results() {
//	printf("**** START PERFORMANCE STATS ****\n");
	std::vector<PProfile> sorted_perfs;
	for (auto& perf : perf_map) {
		PProfile profile;
		profile.func_name = perf.first;
		profile.profile = perf.second;
		sorted_perfs.push_back(profile);
	}
	std::sort(sorted_perfs.begin(), sorted_perfs.end());

    setlocale(LC_NUMERIC, "");
	for (int i = 0; i < sorted_perfs.size(); i++) {
		MethodPerfProfile& mpp = sorted_perfs[i].profile;
		float total = mpp.total_microseconds / 1000.0f;
		float max = mpp.max_microseconds / 1000.0f;
		float avg = total / mpp.total_calls;
		float stddev = sqrt(mpp.qvalue / mpp.total_calls) / 1000.0f;
		float stddev_percentage = (stddev / avg) * 100.0f;
		printf("func %s:\n"
				"\t>> total %'*.2fms"
				"\n\tcalls %'*d"
		        "\n\taverage %'*.2fms"
		        "\n\tmax %'*.2fms"
				"\n\tstd.dev    +-%'.4fms, +-%'.2f%%\n",
				sorted_perfs[i].func_name.c_str(),
				10, total, 13, int(mpp.total_calls),
				11, avg, 15, max,
				stddev, stddev_percentage);
	}
    setlocale(LC_CTYPE, "");
//	printf("**** END PERFORMANCE STATS ****\n");
}

double PerfTimer::average_time(const char* method) {
	MethodPerfProfile& mpp = perf_map[method];
	float total = mpp.total_microseconds / 1000.0f;
	return total / mpp.total_calls;
}

void PerfTimer::clear() {
	perf_map.clear();
}

static PerfTimer __global_timer;

void perf_timer_begin(const char* funcname) {
	__global_timer.begin(funcname);
}

void perf_timer_clear() {
	__global_timer.clear();
}

void perf_timer_end(const char* funcname) {
	__global_timer.end(funcname);
}

double perf_timer_average_time(const char* funcname) {
	return __global_timer.average_time(funcname);
}

void perf_print_results() {
	__global_timer.print_results();
}

