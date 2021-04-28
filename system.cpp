#include "rar.hpp"

static int SleepTime = 0;

void InitSystemOptions(int SleepTime)
{
	::SleepTime = SleepTime;
}


#ifndef SFX_MODULE
void SetPriority(int Priority)
{
}
#endif


void Wait()
{
}
