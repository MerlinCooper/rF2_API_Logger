#include "EnduranceRaceTime.h"

#include <afxwin.h>

EnduranceRaceTime::EnduranceRaceTime(DWORD maxTimeMinutes){
	//Create Worker Thread which suspends x min and than calls CheckRegulation
	maxDrivingTime = 60 * maxTimeMinutes*1E3;
	pMonitorThread = AfxBeginThread(&WaitThread, this);
}

EnduranceRaceTime::~EnduranceRaceTime(){
	//Stop worker Thread if instance of class is deleted before wait time ended
	if (pMonitorThread) {
		::TerminateThread((HANDLE)*pMonitorThread, 0);
	}
}


void EnduranceRaceTime::SetPenalty(bool penalty ) {
	CSingleLock lock(&c_s);
	lock.Lock();

	bPenalty = penalty;

	lock.Unlock();
}

bool EnduranceRaceTime::CheckPenalty() {
	bool retVal;
	CSingleLock lock(&c_s);
	lock.Lock();

	retVal = bPenalty;

	lock.Unlock();
	return retVal;
}



UINT WaitThread(LPVOID param){
	//if no valid parameter is provided return immediately and abort thread with error
	if (NULL == param) return 1;

	//Suspend thread for the total allowed race time per driver after time expired call the CheckRegulation Function
	EnduranceRaceTime *pTime = (EnduranceRaceTime*)param;
	//while (true) {
		Sleep(pTime->maxDrivingTime);
		pTime->SetPenalty();
	//}
	return 0;
}