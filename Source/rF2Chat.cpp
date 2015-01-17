////////////////////////////////////////////////////////////////////////////
//	
//	Functions and global variables to access rF2 ChatWindow and 
//	Server Messages
//
////////////////////////////////////////////////////////////////////////////

#include "rF2Chat.hpp"
#include <afxwin.h>
#include <string>


// handle to the controls
HWND hwndSendChatButton = NULL;
HWND hwndChatEdit = NULL;

// rFactor process ID
DWORD dwRFprocessID = 0;

CListBox *pListBox = NULL;

//BEGIN_MESSAGE_MAP(rF2Chat, CListBox)


BOOL CALLBACK EnumProcInit(HWND hWnd, LPARAM lParam)
{
	TCHAR title[500];
	ZeroMemory(title, sizeof(title));

	GetWindowText(hWnd, title, sizeof(title) / sizeof(title[0]));


	DWORD dwProcessID;
	GetWindowThreadProcessId(hWnd, &dwProcessID);
	if (dwProcessID == dwRFprocessID)
	{
		// found one of four windows: check if it's the server one ("Game Name:" can only be found in that window)
		HWND dlgWnd = FindWindowEx(hWnd, NULL, "Static", "Game Name:");
		if (dlgWnd)
		{
			// find Send Chat button and edit chat is two below
			hwndSendChatButton = FindWindowEx(hWnd, NULL, "Button", "Send Chat");
			hwndChatEdit = GetNextWindow(hwndSendChatButton, GW_HWNDPREV);
			hwndChatEdit = GetNextWindow(hwndChatEdit, GW_HWNDPREV);

			// done
			HWND hwndDlg = GetParent(hwndSendChatButton);
			hwndDlg = GetParent(hwndChatEdit);

			//Create Listbox Class from Dialog Handle
			HWND hwdCtrl = GetDlgItem(hwndDlg, 1034);
			pListBox = (CListBox*)CListBox::FromHandle(hwdCtrl);

			return FALSE;
		}
	}
	return TRUE;
}

rF2Chat::rF2Chat() {

}


// Checks Server Chat Windows for Message "[name] left the race"
// returns true when last server message is that whether left
// driverName contains name of driver
bool rF2Chat::CheckDriverLeftRace(CString& driverName) {

	//Check whether a driver left the game
	if (pListBox) {
		int count = pListBox->GetCount();
		if ((count <= 0) || (lastCnt == count)) return false;

		CString tmp;
		pListBox->GetText(count - 1, tmp);
		int pos = tmp.Find(_T(" left the race"));
		lastCnt = count;

		if (-1 != pos) {									//substring found ?
			driverName = tmp.Left(pos);
			
			return true;
		}

		////if (lastCnt < count) {
		//	CString tmp1, tmp2;
		//	lastCnt = count;
		//	
		//	tmp2 = driverName + _T(" left the race");
		//	if (0 == tmp1.Compare(tmp2))
		//		bDriverLeftRace = true;
		//	if (tmp1 == driverName + _T(" left the race"))
		//		bDriverLeftRace = true;
		////}


		//CString tmpMsg = driverName + _T(" left the race");
		//
		//pListBox->GetText(0, tmp);
		//int index = pListBox->FindStringExact(0, tmpMsg);
		//if ((LB_ERR != index) ) {
		//	bDriverLeftRace = true;

		//}


		//int cnt = pListBox->GetCount();
		//if ((LB_ERR != cnt) && cnt > lastCnt){
		//	//otherwise search list for vehicle in DRS Event List
		//	CString txt;
		//	lastCnt = cnt;
		//	pListBox->GetText(lastCnt - 1, txt);
		//	int found = txt.Find(_T(" left the race"));
		//	if (-1 != found) {
		//		bDriverLeftRace = true;					//remember that driver did leave the game - will be deleted later within the UpdateScoring function
		//		pListBox->AddString("Driver removed");
		//	}
		//}
	}
	return false;
}

void rF2Chat::SendChatMessage(const std::string& strMsg){
	if (hwndSendChatButton && hwndChatEdit) {
		// save what the user was typing
		char szCurrText[100]; 
		GetWindowText(hwndChatEdit, szCurrText, 100);

		SetWindowText(hwndChatEdit, strMsg.c_str());
		::PostMessage(hwndSendChatButton, WM_KEYDOWN, VK_SPACE, 0);
		::PostMessage(hwndSendChatButton, WM_KEYUP, VK_SPACE, 0);

		// put back the message if there was one
		if (strlen(szCurrText))
		{
			Sleep(20);
			SetWindowText(hwndChatEdit, szCurrText);
		}
	}
}