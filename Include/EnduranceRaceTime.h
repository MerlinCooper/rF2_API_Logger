#ifndef _ENDURANCERACETIME_H
#define _ENDURANCERACETIME_H

#include <afxwin.h>
#include <afxmt.h>




class Regulation {
public:
	virtual bool CheckPenalty() { return false; };
	virtual void SetPenalty(bool penalty = true){};
	//virtual void ExecutePenalty(){};
};


UINT WaitThread(LPVOID param);

class EnduranceRaceTime:public Regulation
{
public:
	EnduranceRaceTime(DWORD maxTime);
	~EnduranceRaceTime();
	
	bool CheckPenalty();
	void SetPenalty(bool penalty = true);
	//void ExecutePenalty()
	friend UINT WaitThread(LPVOID param);  
protected:
	DWORD		maxDrivingTime = 4500000; //default value 75 min = 75*60*1E3 ms;
private:
	CWinThread*	pMonitorThread;
	CCriticalSection c_s;
	bool bPenalty = false;
	
};

#endif