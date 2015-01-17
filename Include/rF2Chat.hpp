#ifndef _RF2CHATWND_H
#define _RF2CHATWND_H

#include <afxwin.h>
#include <string>

class rF2Chat {
public: 
	rF2Chat();
	bool CheckDriverLeftRace(CString& driverName);
	void SendChatMessage(const std::string& strMsg);
	
protected:
	friend BOOL CALLBACK EnumProcInit(HWND hWnd, LPARAM lParam);
private:
	int lastCnt = 0;
	DWORD rF2ProcID;
};

#endif