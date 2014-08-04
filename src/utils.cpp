#include "app.h"

#if defined WIN32 || defined _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

QByteArray removeHTTPHeader(QByteArray data,QString header)
{
    int i1 = data.indexOf(header);
    if (i1==-1) return data;
    int i2 = data.indexOf("\n", i1);
    if (i2==-1) return data;
    data.remove(i1, i2-i1+1);
    return data;
}

float timestampNow()
{
    time_t newtime;
#if defined WIN32 || defined _WIN32
    newtime = GetTickCount();
#elif defined __APPLE__
    timeval time;
    gettimeofday(&time, NULL);
    newtime = (time.tv_sec * 1000) + (time.tv_usec / 1000);
#else
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    newtime = tp.tv_sec*1000 + tp.tv_nsec/1000/1000;
#endif
    return (float)(((float)(newtime - app.startTimestamp))/(float)1000); // Seconds since application start (startTimestamp)
}
