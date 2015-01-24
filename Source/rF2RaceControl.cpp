#include "rF2RaceControl.hpp"
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
//#include <Windows.h>
#include <afxwin.h>
#include <atlstr.h>
#include "resource.h"
#include "rF2Chat.hpp"


// GLOBAL variables
const std::string version_number("rF2RaceControl PlugIn 0.7.3");


// plugin information

extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()						{ return(version_number.c_str()); }

extern "C" __declspec( dllexport )
	PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }

extern "C" __declspec( dllexport )
	int __cdecl GetPluginVersion()                         { return( 4 ); } // InternalsPluginV04 functionality

extern "C" __declspec( dllexport )
	PluginObject * __cdecl CreatePluginObject()            { return( (PluginObject *) new rF2RaceControl_Main ); }

extern "C" __declspec( dllexport )
	void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( (rF2RaceControl_Main *) obj ); }

//namespaces
using namespace std;

extern DWORD dwRFprocessID;

rF2RaceControl_Main::rF2RaceControl_Main() {
#ifdef _DEBUG
MessageBox(NULL, "rF2RaceControl_SetEnvironment is being loaded (DEBUG Version)", "Debug Message", MB_OK); 
#endif
};


void rF2RaceControl_Main::UpdateScoring(const ScoringInfoV01 &info){
	
	//check and update driver list
	if (NULL != info.mVehicle) {
		//bool bDriverLeftRace = false;
		
		VehicleScoringInfoV01 *ptrVeh = info.mVehicle;

		CString driverName;
		if (chatWnd.CheckDriverLeftRace(driverName)) {										//a driver left the race
			DRSEvent toFind(driverName.GetString(), "");
			POSITION pos = mTeams.Find(toFind);
			if (NULL != pos)
				mTeams.RemoveAt(pos);
			}

		for (long vehCnt = info.mNumVehicles; vehCnt > 0; vehCnt--) {
			DRSEvent vehicleToSearch(ptrVeh->mDriverName, ptrVeh->mVehicleName);
			POSITION mIndex = mTeams.Find(vehicleToSearch);

			if (NULL == mIndex) {													//new driver not yet in list
				time(&vehicleToSearch.srvMsgLastSent);								//store when last server welcome was sent.
				mTeams.AddTail(vehicleToSearch);									//add driver to list
				if (bDoSrvMsg) WriteSrvWelcomMsg(ptrVeh->mDriverName);				//write Server Welcome Message in Chat Window
			} else {
				DRSEvent* pDriver = &mTeams.GetAt(mIndex);

				//----------------------
				//repeat Srv Message
				if (srvMsgInt != 0){
					//Repeat Wellcome Message (not for VEC)
					if (ptrVeh->mInPits) {												//if driver is in pits 
						// check whether server welcome message supposed to be re-sent
						time(&now);
						if (difftime(now, pDriver->srvMsgLastSent) > srvMsgInt) {		//send server welcome message again after configured time
							pDriver->UpdateTime();
							WriteSrvWelcomMsg(pDriver->driverName.c_str(), false);		//write Server Welcome Message in Chat Window
						}
					}
				}
				//----------------------
				//check max driving time
				if (bApplyEndurance_Rule) {
					CheckMaxDrivingTime(pDriver, ptrVeh->mInPits);
					if ((NULL != pDriver->pRaceTimer) && pDriver->pRaceTimer->CheckPenalty()) {
						SetPenalty(penaltyEndurance, pDriver->driverName, ENDURANCE_MAX_STINT_TIME);
						delete pDriver->pRaceTimer; 
						pDriver->pRaceTimer = NULL; 
						pDriver->bStintStarted = false;
					}

				}

			}
			ptrVeh++;
		}
	}

	//Write one chat message per call 
	if (m_MessageQueue.size()) 
	{
		chatWnd.SendChatMessage(m_MessageQueue.front());
		m_MessageQueue.pop_front();
	}
}

//---------------------------------------------------------------------------------------------------------
void rF2RaceControl_Main::UpdateTelemetry(const TelemInfoV01 &info){

	

	if (bApplyDTM_Rules) {
		if ((info.mCurrentSector & 0x80000000) ||	//if vehicle in pits (indicated by sign bit) or 
			(info.mRearFlapLegalStatus != 2)) 		//rearflap is not allowed by game
			return;									//return because there is nothing to do - no abuse of DRS possible
		
		//otherwise search list for vehicle in DRS Event List
		DRSEvent vehicleToSearch(info);
		driverIterator foundVehicle = find(m_DriverList.begin(), m_DriverList.end(), vehicleToSearch);

		//if vehicle is being found - look at DRS status and amount of activiations per lap 
		if (foundVehicle != m_DriverList.end()) {
			if (info.mLapNumber != foundVehicle._Ptr->lastLap) {			//new Lap started ?
				foundVehicle._Ptr->lastLap = info.mLapNumber;				//store current lap
				if (foundVehicle._Ptr->counterDRS > 1) {					//DRS used more than once per lap ?
					SetPenalty(penaltyDRS, foundVehicle._Ptr->driverName, DTM_DRS);
				}
				foundVehicle._Ptr->counterDRS = 0;							//reset DRS counter because of new Lap
			}

			//if DRS status has changed increase DRS counter
			if (foundVehicle._Ptr->mRearFlapActivated != info.mRearFlapActivated) {
				//Fahrzeug ist in Liste - nun überprüfen ob sich DRS status geändert hat. 
				foundVehicle._Ptr->mRearFlapActivated = info.mRearFlapActivated;
				if (info.mRearFlapActivated == 0x01)	{													//DRS active
					foundVehicle._Ptr->counterDRS = foundVehicle._Ptr->counterDRS + 1;
				}
			}
		}
	}
}

//rF2RaceControl_Main class definition -test
void rF2RaceControl_Main::SetEnvironment( const EnvironmentInfoV01 &info )       
{
	if (environmentDone) return;
	environmentDone = true;

	

	// get rF process ID
	dwRFprocessID = GetCurrentProcessId();

	//open Logfile
	if (!logFileStewards.is_open()) {
		profilePath = info.mPath[1];
		//unsigned found = ;
		profilePath = profilePath.substr(0, profilePath.find_last_of("/\\"));

		//open log file
		logFilePath = profilePath;
		logFilePath.append("\\rF2RaceControl_Main_Log.txt");
		logFileStewards.open(logFilePath.c_str(), ios::out | ios::app);
	}

	profilePath = info.mPath[1];
	//unsigned found = 
	profilePath = profilePath.substr(0,profilePath.find_last_of("/\\"));

	////open log file
	//logFilePath = profilePath;
	//logFilePath.append("\\rF2RaceControl_Main_Log.txt");
	//logFile.open(logFilePath.c_str());


	iniConfigFilename = profilePath;
	iniConfigFilename.append("\\rF2RaceControl.ini");

	//Read INI File 
	ReadIniFile(iniConfigFilename);	
}

void rF2RaceControl_Main::StartSession()
{
	// find the handle to the drivers list
	EnumWindows(EnumProcInit, 0);

	//read message from resource file
	for (int i = 0; i < 5; i++) {
		srvMessageLine[i].LoadString(IDS_SRV_WELCOME_LINE1 + i);
	}

	ostringstream oss;
	oss << version_number << " (C) Merlin Cooper\n";

	m_MessageQueue.push_back(oss.str());
	//SendChatMessage(oss.str());
}

void rF2RaceControl_Main::EndSession(){					// session ended
	mTeams.RemoveAll();
}


/*void rF2RaceControl_Main::SendChatMessage(const std::string& strMsg) const
{
	if (hwndSendChatButton && hwndChatEdit) {
		// save what the user was typing
		char szCurrText[100]; GetWindowText(hwndChatEdit, szCurrText, 100);
		std::string str = szCurrText;

		//// parse for \n and post the message to chat Window
		//basic_string <char>::size_type posLF_prev = 0, posLF_cur = strMsg.find("\n");
		//do{			//write substrings until end of string reached
		//	CString msgLine (strMsg.substr(posLF_prev, posLF_cur).c_str());			//Here is line of the string to write to chat window
		//	//
		//	posLF_prev = posLF_cur+2;												//next search starts 2 characters later
		//	posLF_cur = strMsg.find("\n", posLF_prev);
		//} while (posLF_cur != string::npos);

		SetWindowText(hwndChatEdit, strMsg.c_str());
		::PostMessage(hwndSendChatButton, WM_KEYDOWN, VK_SPACE, 0);
		::PostMessage(hwndSendChatButton, WM_KEYUP, VK_SPACE, 0);

		// put back the message if there was one
		if (str.length())
		{
			Sleep(20);
			SetWindowText(hwndChatEdit, szCurrText);
		}
	}
}*/


void  rF2RaceControl_Main::ReadIniFile(const std::string& iniFileName){
	//Read from Ini-File whether DTM Rules should be activated
	char buffer[512];
	ostringstream oss;

	//Read Server Message parameter from INI File
	GetPrivateProfileString("General","SrvMsg","(C) Merlin Cooper 2015",buffer,sizeof(buffer)-1,iniFileName.c_str());
	oss << buffer << "\n";
	serverWelcomeMessage.assign(oss.str());
	switch (GetPrivateProfileInt("General", "Activate_SrvMsg", 0, iniFileName.c_str())){
	case 0:
		bDoSrvMsg = false; break;
	case 1: bDoSrvMsg = true;
	}
	
	srvMsgInt = GetPrivateProfileInt("General", "SrvMsg_Interval", 0, iniFileName.c_str());
	
	//reset string stream	
	oss.str(""); oss.clear();

	switch (GetPrivateProfileInt("DTM_Rules", "Regulation_Active", 0, iniFileName.c_str())){
	case 0: 
		bApplyDTM_Rules = false; break;
	case 1: bApplyDTM_Rules = true;
	}

	if (bApplyDTM_Rules) {			//read further values just in case DTM rules are activated
		GetPrivateProfileString("DTM_Rules","Info","DTM DRS RULES ACTIVE - ONLY ONE TIME ACTIVATION OF DRS PER LAP ALLOWED",buffer,sizeof(buffer)-1,iniFileName.c_str());
		oss << buffer << "\n";
		DTM_InfoMsg.append(oss.str());
		m_MessageQueue.push_back("DTM DRS RULES ACTIVE");

		//read type of penalty to be given if DTM rule is violated
		switch (GetPrivateProfileInt("DTM_Rules", "Penalty", 0, iniFileName.c_str())){
		case 1: penaltyDRS = DRIVE_THROUGH; break;
		case 2: penaltyDRS = STOP_GO; break;
		default:
			penaltyDRS = WARNING;
		}
	}

	//reset string stream	
	oss.str(""); oss.clear();

	switch (GetPrivateProfileInt("YELLOW_FLAG", "Regulation_Active", 0, iniFileName.c_str())){
	case 0: bApplyYellowFlag_Rule = false; break;
	case 1: bApplyYellowFlag_Rule = true;
	};
	if (bApplyYellowFlag_Rule) {GetPrivateProfileString("YELLOW_FLAG","Info","YELLOW FLAG RULES ACTIVE - REDUCE SPEED IN CASE OF YELLOW FLAG",buffer,sizeof(buffer)-1,iniFileName.c_str());
		oss << buffer << "\n";
		YellowFlag_InfoMsg.append(oss.str());;
		m_MessageQueue.push_back("YELLOW FLAG RULES ACTIVE (currently not implemented)");
	}

	//reset string stream	
	oss.str(""); oss.clear();

	//Read all necessary values for Endurance Timing Regulation
	switch (GetPrivateProfileInt("ENDURANCE", "Regulation_Active", 0, iniFileName.c_str())){
	case 0: bApplyEndurance_Rule = false; break;
	case 1: bApplyEndurance_Rule = true;
	};
	if (bApplyEndurance_Rule) {
		GetPrivateProfileString("ENDURANCE","Info","ENDURANCE: Maximumg Driving Time rule applyed",buffer,sizeof(buffer) - 1,iniFileName.c_str());
		//read type of penalty to be given if DTM rule is violated
		switch (GetPrivateProfileInt("ENDURANCE", "Penalty", 0, iniFileName.c_str())){
		case 1: penaltyEndurance = DRIVE_THROUGH; break;
		case 2: penaltyEndurance = STOP_GO; break;
		default:
			penaltyEndurance = WARNING;
		}
		//read value for maximum allowed driving time 
		maxRacingTime = GetPrivateProfileInt("ENDURANCE", "MaxRacingTime", 75, iniFileName.c_str());
		stopGoTime = GetPrivateProfileInt("ENDURANCE", "StopGoTime", 60, iniFileName.c_str());
		if (stopGoTime < 0 || stopGoTime > 60) stopGoTime = 60;

		oss << buffer << "\n";
		maxDrivingTime_InfoMsg.append(oss.str());;
		m_MessageQueue.push_back(maxDrivingTime_InfoMsg.c_str());
	}
}

void  rF2RaceControl_Main::WriteIniFile(const std::string& iniFileName){

}

void rF2RaceControl_Main::SetPenalty(penalty_type penalty, std::string &driverName, rule_type rule, UINT iStopGoTime){
	std::ostringstream oss;

	switch (penalty) {
	case DRIVE_THROUGH:
		oss << "/addpenalty -1 " << driverName << "\n";							//chat text for drive through
		break;
	case STOP_GO:
		oss << "/addpenalty " << iStopGoTime << " " << driverName << "\n";		//chat text for 60 sec stop and go
		break;
	case WARNING:
		oss << "/w " << driverName << "WARNING" << "\n";						//chat text for warning
		break;
	}

	m_MessageQueue.push_back(oss.str());				//Output to chat window
	logFileStewards << oss.str();						//Output to log file for stewards
	
	oss.str(""); oss.clear();

	switch (rule) {
	case DTM_DRS:
		oss << "/w" << driverName << "DTM DRS RULE VIOLATED" << "\n";		//chat text for warning
		break;
	case YELLOW_FLAG:
		oss << "/w" << driverName << "YELLOW FLAG VIOLATED" << "\n";		//chat text for warning
		break;
	case ENDURANCE_MAX_STINT_TIME:
		oss << "/w" << driverName << "MAXIMUM DRIVING TIME PER STINT EXCEEDED" << "\n";		//chat text for warning
	}

	m_MessageQueue.push_back(oss.str());				//Output to chat window
	logFileStewards << oss.str();						//Output to log file for stewards
	oss.flush();


}

void rF2RaceControl_Main::DeactivateScreen(const ScreenInfoV01 &info) {

}

void rF2RaceControl_Main::ReactivateScreen(const ScreenInfoV01 &info) {

}

void rF2RaceControl_Main::ExitRealtime() {

}

void rF2RaceControl_Main::WriteSrvWelcomMsg(const CString driverName, bool longMsg ){
	CString temp;

	if (longMsg){
		for (int i = 0; i < 5; i++) {
			if (i == 0) {
				temp.Format(srvMessageLine[i], driverName, driverName);
			}
			else {
				temp.Format(srvMessageLine[i], driverName);
			}
			m_MessageQueue.push_back((LPCSTR)temp);
		}
	} else {
		temp.Format(srvMessageLine[1], driverName);
	}

}


void rF2RaceControl_Main::CheckMaxDrivingTime(DRSEvent *pDriver, const bool bInPits){
	if (!bInPits && !pDriver->bStintStarted)
		pDriver->StartMonitoringMaxDrivingTime(maxRacingTime);
}

DRSEvent::~DRSEvent(){
	if (pRaceTimer) delete pRaceTimer;
}

void DRSEvent::StartMonitoringMaxDrivingTime(DWORD maxTime){
	if (pRaceTimer == NULL) {
		pRaceTimer = new EnduranceRaceTime(maxTime);
		bStintStarted = true;
	}
}