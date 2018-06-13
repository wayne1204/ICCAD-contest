#ifndef MY_USAGE_H
#define MY_USAGE_H

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sys/times.h>
#include <sys/resource.h>

using namespace std;

#undef MYCLK_TCK
#define MYCLK_TCK sysconf(_SC_CLK_TCK)

class MyUsage
{
  public:
    MyUsage() { 
        _initMem = checkMem();
        _currentTick = checkTick();
        _periodUsedTime = _totalUsedTime = 0.0;
    }

    void report() {
        cout << endl;
        setTimeUsage();
        cout << "Period time used : " << setprecision(4)
             << _periodUsedTime << " seconds" << endl;
        cout << "Total time used  : " << setprecision(4)
             << _totalUsedTime << " seconds" << endl;
        setMemUsage();
        cout << "Total memory used: " << setprecision(4)
             << _currentMem << " M Bytes" << endl;
        cout << endl;
    }

  private:
    // for Memory usage (in MB)
    double _initMem;
    double _currentMem;

    // for CPU time usage
    double _currentTick;
    double _periodUsedTime;
    double _totalUsedTime;

    // private functions
    double checkMem() const
    {
        struct rusage usage;
        if (0 == getrusage(RUSAGE_SELF, &usage))
            return usage.ru_maxrss / double(1 << 10); // KBytes
        else
            return 0;
    }
    double checkTick() const
    {
        tms tBuffer;
        times(&tBuffer);
        return tBuffer.tms_utime;
    }
    void setMemUsage() { _currentMem = checkMem() - _initMem; }
    void setTimeUsage()
    {
        double thisTick = checkTick();
        _periodUsedTime = (thisTick - _currentTick) / double(MYCLK_TCK);
        _totalUsedTime += _periodUsedTime;
        _currentTick = thisTick;
    }
};

#endif // MY_USAGE_H
