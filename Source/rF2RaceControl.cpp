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
const std::string version_number("rF2RaceControl PlugIn 0.7.2");


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
MessageBox(NULL, "rF2RaceControl_SetEnvironment is being loaded (DEBUG Version)", "Debug Message", MB_OK); 
};


void rF2RaceControl_Main::UpdateScoring(const ScoringInfoV01 &info){


	if (NULL != info.mVehicle) {
		bool bDriverLeftRace = false;
		
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
			//if (NULL != info.mVehicle)
			//bDriverLeftRace = chatWnd.CheckDriverLeftRace(info.mVehicle->mDriverName);

			// compare to the previous driver list


			POSITION mIndex = mTeams.Find(vehicleToSearch);

			if (NULL == mIndex) {													//new driver not yet in list
				mTeams.AddTail(vehicleToSearch);									//add driver to list
				WriteSrvWelcomMsg(ptrVeh->mDriverName);								//write Server Welcome Message in Chat Window
			}
			//else if (chatWnd.CheckDriverLeftRace(ptrVeh->mDriverName)) {		//driver already in list and did leave the game?
			//	mTeams.RemoveAt(mIndex);
			//}
			//else {																	//check whether driver violated a rule

			//}


			//if (bDriverLeftRace) {
			//	m_DriverList.erase(foundVehicle);
			//	//POSITION mIndex = mTeams.FindIndex(vehicleToSearch);
			//	if (NULL != mIndex)
			//		mTeams.RemoveAt(mIndex);
			//	bDriverLeftRace = false;											//Set to false because one driver already deleted from list
			//}
			//else {
			//	if (foundVehicle == m_DriverList.end())	{							//if first time that UpdateScoring is called for specific vehicle/driver combination
			//		WriteSrvWelcomMsg(ptrVeh->mDriverName);			//write Server Welcome Message in Chat Window
			//		m_DriverList.insert(m_DriverList.begin(), vehicleToSearch);		//add vehicle to the list
			//		mTeams.AddTail(vehicleToSearch);
			//	}

			//}

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
	GetPrivateProfileString(
		"General",
		"SrvMsg",
		"(C) Merlin Cooper 2015",
		buffer,
		sizeof(buffer)-1,
		iniFileName.c_str());
	oss << buffer << "\n";
	serverWelcomeMessage.assign(oss.str());
	GetPrivateProfileInt("General", "SrvMsg_Interval", 0, iniFileName.c_str());
	
	//reset string stream	
	oss.str("");
	oss.clear();

	switch (GetPrivateProfileInt("DTM_Rules", "Regulation_Active", 0, iniFileName.c_str())){
	case 0: 
		bApplyDTM_Rules = false; break;
	case 1: bApplyDTM_Rules = true;
	};
	if (bApplyDTM_Rules) {			//read further values just in case DTM rules are activated
		GetPrivateProfileString(
			"DTM_Rules",
			"Info",
			"DTM DRS RULES ACTIVE - ONLY ONE TIME ACTIVATION OF DRS PER LAP ALLOWED",
			buffer,
			sizeof(buffer)-1,
			iniFileName.c_str());
		
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

	oss.seekp(ios::beg);			//set pointer in outputstream to begin

	switch (GetPrivateProfileInt("YELLOW_FLAG", "Regulation_Active", 0, iniFileName.c_str())){
	case 0: bApplyYellowFlag_Rule = false; break;
	case 1: bApplyYellowFlag_Rule = true;
	};
	if (bApplyYellowFlag_Rule) {
		GetPrivateProfileString(
			"YELLOW_FLAG",
			"Info",
			"YELLOW FLAG RULES ACTIVE - REDUCE SPEED IN CASE OF YELLOW FLAG",
			buffer,
			sizeof(buffer)-1,
			iniFileName.c_str());
		oss << buffer << "\n";
		YellowFlag_InfoMsg.append(oss.str());;
		m_MessageQueue.push_back("YELLOW FLAG RULES ACTIVE (currently not implemented");
	}
}

void  rF2RaceControl_Main::WriteIniFile(const std::string& iniFileName){

}

void rF2RaceControl_Main::SetPenalty(penalty_type penalty, std::string &driverName, rule_type rule){
	std::ostringstream oss;

	switch (penalty) {
	case DRIVE_THROUGH:
		oss << "/addpenalty -1 " << driverName << "\n";			//chat text for drive through
		//m_MessageQueue.push_back(oss.str());					//Output to chat window
	case STOP_GO:
//		oss << "/addpenalty " << /* Sekunden Stop/Go" */ << driverName << "\n";			//chat text for 60 sec stop and go
		break;
	case WARNING:
		oss << "/w " << driverName << "WARNING" << "\n";		//chat text for 60 sec stop and go
		break;
	}

	m_MessageQueue.push_back(oss.str());				//Output to chat window
	logFileStewards << oss.str();						//Output to log file for stewards
	oss.flush();

	switch (rule) {
	case DTM_DRS:
		oss << "/w" << driverName << "DTM DRS RULE VIOLATED" << "\n";		//chat text for warning
		break;
	case YELLOW_FLAG:
		oss << "/w" << driverName << "YELLOW FLAG VIOLATED" << "\n";		//chat text for warning
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

void rF2RaceControl_Main::WriteSrvWelcomMsg(const CString driverName){
	CString temp;
	for (int i = 0; i < 5; i++) {
		if (i == 0) {
			temp.Format(srvMessageLine[i], driverName, driverName);
		}
		else {
			temp.Format(srvMessageLine[i], driverName);		
		}
		m_MessageQueue.push_back((LPCSTR)temp);
	}

}


