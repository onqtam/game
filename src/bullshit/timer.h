
/// This is a stop-watch style timer inspired by QElapsedTimer in Qt.
/// It is high-precision and cross-platform.
/// This class is not thread safe or reentrant!
/// On some systems the nano precision may overflow after more than an hour.
class ElapsedTimer {
public:
	/// The constructor starts the timer. Use restart() to control starting point.
	ElapsedTimer();
	
	/// Returns the highest available timer precision or EP_INVALID if the timer is unusable.
	ElapsedPrecision precision() const;
	
	/// Returns the maximum timer resolution in nanoseconds (rounded to integer).
	int64 nanosecResolution() const;
	
	/// Reset the "start time". This affects both *SinceStart() and *SinceLast()
	/// @param retPrecision The precision of the returned value
	/// @return Time since construction or last restart()
	int64 restart(ElapsedPrecision retPrecision = EP_NANO);
	
	/// Return milli/micro/nanoseconds since the timer was started.
	int64 sinceStart(ElapsedPrecision retPrecision = EP_NANO) const;
	
	/// Return milli/micro/nanoseconds since the timer was started or this function was last called.
	int64 sinceLast(ElapsedPrecision retPrecision = EP_NANO);
	
private:
#ifdef _WIN32
	LARGE_INTEGER freq;
	LARGE_INTEGER start;
	LARGE_INTEGER last;
#elif defined __APPLE__
	uint64 start;
	uint64 last;
	double toMilli;
	double toMicro;
	double toNano;
#else // POSIX
	struct timespec start;
	struct timespec last;
	struct timespec resolution;
#endif
};





#ifdef _WIN32
//#  include <winbase.h>
#elif defined __APPLE__
#  include <mach/mach_time.h>
#else // POSIX
#  include <sys/time.h>
#endif

#ifdef _WIN32

ElapsedTimer::ElapsedTimer()
{
	freq.QuadPart = 0LL;
	start.QuadPart = 0LL;
	last.QuadPart = 0LL;
	
	bool success = QueryPerformanceFrequency(&freq);
	success &= QueryPerformanceCounter(&start);
	last = start;

	if ( ! success) {
		// make sure precision() will return EP_INVALID
		freq.QuadPart = 0LL;
	}
}

ElapsedPrecision ElapsedTimer::precision() const
{
	if (freq.QuadPart >= 1000000000LL)
		return EP_NANO;
	if (freq.QuadPart >= 1000000LL)
		return EP_MICRO;
	if (freq.QuadPart >= 1000LL)
		return EP_MILLI;
	return EP_INVALID;
}

int64 ElapsedTimer::nanosecResolution() const
{
	if (freq.QuadPart == 0LL)
		return 0LL;
	return 1000000000LL / freq.QuadPart;
}

int64 ElapsedTimer::restart(ElapsedPrecision retPrecision)
{
	if (freq.QuadPart == 0LL)
		return 0LL;
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	int64 retval = now.QuadPart - start.QuadPart;
	last = start = now;
	switch (retPrecision) {
		case EP_NANO: return retval * 1000000000LL / freq.QuadPart;
		case EP_MICRO: return retval * 1000000LL / freq.QuadPart;
		case EP_MILLI: return retval * 1000LL / freq.QuadPart;
	}
	return 0LL;
}

int64 ElapsedTimer::sinceStart(ElapsedPrecision retPrecision) const
{
	if (freq.QuadPart == 0LL)
		return 0LL;
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	int64 retval = now.QuadPart - start.QuadPart;
	switch (retPrecision) {
		case EP_NANO: return retval * 1000000000LL / freq.QuadPart;
		case EP_MICRO: return retval * 1000000LL / freq.QuadPart;
		case EP_MILLI: return retval * 1000LL / freq.QuadPart;
	}
	return 0LL;
}

int64 ElapsedTimer::sinceLast(ElapsedPrecision retPrecision)
{
	if (freq.QuadPart == 0LL)
		return 0LL;
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	now.QuadPart -= last.QuadPart;
	last.QuadPart += now.QuadPart;
	switch (retPrecision) {
		case EP_NANO: return now.QuadPart * 1000000000LL / freq.QuadPart;
		case EP_MICRO: return now.QuadPart * 1000000LL / freq.QuadPart;
		case EP_MILLI: return now.QuadPart * 1000LL / freq.QuadPart;
	}
	return 0LL;
}

#elif defined __APPLE__

ElapsedTimer::ElapsedTimer()
{
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	
	if (timebase.denom == 0LL) {
		// precision() will return EP_INVALID
		toNano = toMicro = toMilli = 0.0;
	}
	else {
		toNano = timebase.numer / timebase.denom;
		toMicro = toNano * 1000.0;
		toMilli = toMicro * 1000000.0;
	}
	
	start = mach_absolute_time();
	last = start;
}

ElapsedPrecision ElapsedTimer::precision() const
{
	if (toNano > .999)
		return EP_NANO;
	if (toMicro > .999)
		return EP_MICRO;
	if (toMilli > .999)
		return EP_MILLI;
	return EP_INVALID;
}

int64 ElapsedTimer::nanosecResolution() const
{
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	if (timebase.numer)
		return timebase.denom / timebase.numer;
	return 0LL;
}

int64 ElapsedTimer::restart(ElapsedPrecision retPrecision)
{
	int64 now = mach_absolute_time();
	int64 retval = now - start;
	last = start = now;
	switch (retPrecision) {
		case EP_NANO: return (int64)(retval * toNano);
		case EP_MICRO: return (int64)(retval * toMicro);
		case EP_MILLI: return (int64)(retval * toMilli);
	}
	return 0LL;
}

int64 ElapsedTimer::sinceStart(ElapsedPrecision retPrecision) const
{
	int64 now = mach_absolute_time();
	int64 retval = now - start;
	switch (retPrecision) {
		case EP_NANO: return (int64)(retval * toNano);
		case EP_MICRO: return (int64)(retval * toMicro);
		case EP_MILLI: return (int64)(retval * toMilli);
	}
	return 0LL;
}

int64 ElapsedTimer::sinceLast(ElapsedPrecision retPrecision)
{
	int64 now = mach_absolute_time();
	int64 retval = now - last;
	last = now;
	switch (retPrecision) {
		case EP_NANO: retval = (int64)(retval * toNano); break;
		case EP_MICRO: retval = (int64)(retval * toMicro); break;
		case EP_MILLI: retval = (int64)(retval * toMilli); break;
		default: retval = 0LL;
	}
	return retval;
}

#else // POSIX

ElapsedTimer::ElapsedTimer()
{
	memset(&resolution, 0, sizeof(resolution));
	memset(&start, 0, sizeof(start));
	memset(&last, 0, sizeof(last));
	
	int success = clock_getres(CLOCK_MONOTONIC, &resolution);
	success += clock_gettime(CLOCK_MONOTONIC, &start);
	last = start;

	if (success != 0) {
		// make sure precision() will return EP_INVALID
		resolution.tv_nsec = 1000000000;
	}
}

ElapsedPrecision ElapsedTimer::precision() const
{
	if (resolution.tv_nsec == 1)
		return EP_NANO;
	if (resolution.tv_nsec <= 1000)
		return EP_MICRO;
	if (resolution.tv_nsec <= 1000000)
		return EP_MILLI;
	return EP_INVALID;
}

int64 ElapsedTimer::nanosecResolution() const
{
	return resolution.tv_nsec;
}

int64 ElapsedTimer::restart(ElapsedPrecision retPrecision)
{
	struct timespec now;
	now.tv_sec = 0;
	now.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &now);
	int64 retval = (1000000000LL * now.tv_sec + now.tv_nsec) - (1000000000LL * start.tv_sec + start.tv_nsec);
	last = start = now;
	switch (retPrecision) {
		case EP_NANO: return retval;
		case EP_MICRO: return retval / 1000;
		case EP_MILLI: return retval / 1000000;
	}
	return 0;
}

int64 ElapsedTimer::sinceStart(ElapsedPrecision retPrecision) const
{
	struct timespec now;
	now.tv_sec = 0;
	now.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &now);
	int64 retval = (1000000000LL * now.tv_sec + now.tv_nsec) - (1000000000LL * start.tv_sec + start.tv_nsec);
	switch (retPrecision) {
		case EP_NANO: return retval;
		case EP_MICRO: return retval / 1000;
		case EP_MILLI: return retval / 1000000;
	}
	return 0;
}

int64 ElapsedTimer::sinceLast(ElapsedPrecision retPrecision)
{
	struct timespec now;
	now.tv_sec = 0;
	now.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &now);
	now.tv_sec -= last.tv_sec;
	now.tv_nsec -= last.tv_nsec;
	last.tv_sec += now.tv_sec;
	last.tv_nsec += now.tv_nsec;
	switch (retPrecision) {
		case EP_NANO: return (1000000000LL * now.tv_sec + now.tv_nsec);
		case EP_MICRO: return (1000000000LL * now.tv_sec + now.tv_nsec) / 1000;
		case EP_MILLI: return (1000000000LL * now.tv_sec + now.tv_nsec) / 1000000;
	}
	return 0;
}

#endif // OS