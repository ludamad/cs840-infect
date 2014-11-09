/*
 * perf_timer.h:
 *  Provides timing information on a per-method basis.
 *  Based on a global PerfTimer, but does not expose that class.
 *  Public domain (by Adam Domurad)
 */

#ifndef LCOMMON_PERF_TIMER_H_
#define LCOMMON_PERF_TIMER_H_

#include <cstdio>

// Define a cross-platform function name identifier
#ifdef _MSC_VER
#define FUNCNAME __FUNCSIG__
#else
#ifdef __GNUC__
#define FUNCNAME __PRETTY_FUNCTION__
#else
#define FUNCNAME __func__
#endif
#endif

void perf_timer_begin(const char* funcname);
void perf_timer_end(const char* funcname);
double perf_timer_average_time(const char* funcname);
void perf_timer_clear();
void perf_print_results();

struct PerfCount {
    PerfCount(const char* funcname) : funcname(funcname){
        perf_timer_begin(funcname);
    }
    ~PerfCount() {
        perf_timer_end(funcname);
    }
private:
    const char* funcname;
};

struct PerfUnit {
    PerfUnit(const char* unitname) : unitname(unitname){
        perf_timer_clear();
    }
    ~PerfUnit() {
    	printf("-------------- RESULTS FOR %s -----------------\n", unitname);
    	perf_print_results();
        perf_timer_clear();
    	printf("-------------- END RESULTS FOR %s -------------\n", unitname);
    }
private:
    const char* unitname;
};

#define PERF_TIMER() PerfCount __perf_count(FUNCNAME)
#define PERF_TIMER2(name) PerfCount __perf_count(name)
#define PERF_UNIT(unitname) PerfUnit __perf_unit(unitname)


#endif /* LCOMMON_PERF_TIMER_H_ */
