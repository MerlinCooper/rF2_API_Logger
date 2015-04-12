//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//›                                                                         ﬁ
//› Module: Internals Example Header File                                   ﬁ
//›                                                                         ﬁ
//› Description: Declarations for the Internals Example Plugin              ﬁ
//›                                                                         ﬁ
//›                                                                         ﬁ
//› This source code module, and all information, data, and algorithms      ﬁ
//› associated with it, are part of CUBE technology (tm).                   ﬁ
//›                 PROPRIETARY AND CONFIDENTIAL                            ﬁ
//› Copyright (c) 1996-2008 Image Space Incorporated.  All rights reserved. ﬁ
//›                                                                         ﬁ
//›                                                                         ﬁ
//› Change history:                                                         ﬁ
//›   tag.2005.11.30: created                                               ﬁ
//›                                                                         ﬁ
//ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ

#ifndef _RF2RACECONTROL_H
#define _RF2RACECONTROL_H

#include "InternalsPlugin.hpp"
#include "EnduranceRaceTime.h"
#include <set>
#include <vector>
#include <list>
#include <fstream>
#include "rF2Chat.hpp"
#include "C_TCP_Server_Manager.h"
#include "C_Log_Writer.h"
#include "C_TCP_Server_Manager.h"
#include "C_TCP_Server_Sender.h"


//class to encapsulate events for a specific driver (at the moment just number times DRS actived per Lap;
class DRSEvent {
public:
	DRSEvent() { driverName = vehicleName = ""; counterDRS = lastLap = (long)0; };
	~DRSEvent();
	DRSEvent(const TelemInfoV01 &info) { vehicleName = info.mVehicleName; };
	DRSEvent(std::string d_DriverName, std::string d_vehicleName) { driverName = d_DriverName; vehicleName = d_vehicleName;  counterDRS = lastLap = 0; };

	friend bool operator == (const DRSEvent &drsEvent1, const DRSEvent &drsEvent2) { return /*(drsEvent1.vehicleName == drsEvent2.vehicleName) && */(drsEvent1.driverName == drsEvent2.driverName); };
	friend bool operator != (const DRSEvent &drsEvent1, const DRSEvent &drsEvent2) { return /*(drsEvent1.vehicleName != drsEvent2.vehicleName) ||*/ (drsEvent1.driverName != drsEvent2.driverName); };
	//bool compare(const DRSEvent &DRSEvent) { return driverName == DRSEvent.driverName };

	void StartMonitoringMaxDrivingTime(DWORD maxTime);

	std::string		driverName;					//name of driver
	std::string		vehicleName;
	long			counterDRS;					//number of DRS activations per Lap
	unsigned char	mRearFlapActivated = 0x00;	//DRS active - same type as in TelemInfoV01 struct
	long			lastLap;					//number of last lap drivers was running
	
	//Variables used for Server Information every x min. 
	//CTime			lastServerInfoSent;
	void UpdateTime() { time(&srvMsgLastSent); };
	time_t			srvMsgLastSent;

	time_t			startDriving = 0;
	bool			bLastPitStatus = true;
	bool			bStintStarted = false;

	EnduranceRaceTime*	pRaceTimer = NULL;
};

/*class EnduranceRaceTime
{
public:
	EnduranceRaceTime(){};
	~EnduranceRaceTime(){};
};*/


class rF2RaceControl_Main : public InternalsPluginV04{
public: 
	rF2RaceControl_Main();
	~rF2RaceControl_Main(){ logFileStewards.close(); };
	typedef std::vector <DRSEvent>::const_iterator driverIterator;

private:
	enum penalty_type { WARNING, DRIVE_THROUGH, STOP_GO };
	enum rule_type { DTM_DRS, YELLOW_FLAG, ENDURANCE_MAX_STINT_TIME};

	//void Startup(long version);		// game startup
	void StartSession();				// session has started
	void EndSession();					// session ended
	void SetEnvironment( const EnvironmentInfoV01 &info );
	//void EnterRealtime();
	void ExitRealtime();

	void DeactivateScreen(const ScreenInfoV01 &info);			// Window deactivation
	void ReactivateScreen(const ScreenInfoV01 &info);			// Window reactivation

	void ReadIniFile(const std::string& iniFileName);
	void WriteIniFile(const std::string& iniFileName);

	
	bool WantsScoringUpdates() { return(true); }
	void UpdateScoring( const ScoringInfoV01 &info );

	long WantsTelemetryUpdates() { return(2); }				// whether we want telemetry updates (0=no 1=player-only 2=all vehicles)
	void UpdateTelemetry(const TelemInfoV01 &info);			// update plugin with telemetry info

	//void SendChatMessage(const std::string& strMsg) const;
	bool WantsToDisplayMessage(MessageInfoV01 &msgInfo);

	void CheckMaxDrivingTime(DRSEvent *pDriver, const bool bInPits);

	void SetPenalty(penalty_type penalty, std::string &driverName, rule_type rule, UINT iStopGoTime = 0);

	//environment variables
	bool environmentDone = FALSE;

	std::list<std::string> m_MessageQueue;		//chat Window

	std::string messageFilePath;
	std::string logFilePath;
	std::string profilePath;
	std::string iniConfigFilename;
	std::vector<std::string> m_PenaltyText;
	std::vector<DRSEvent> m_DriverList;
	CList <DRSEvent, DRSEvent&> mTeams;

	bool bApplyDTM_Rules = false;
	bool bApplyYellowFlag_Rule = false;
	bool bDoSrvMsg = false;
	
	std::string DTM_InfoMsg;
	std::string YellowFlag_InfoMsg;
	penalty_type penaltyDRS;
	penalty_type penaltyYellowFlag;

	//Endurance
	std::string maxDrivingTime_InfoMsg;
	bool bApplyEndurance_Rule = false;
	penalty_type penaltyEndurance = DRIVE_THROUGH;
	UINT stopGoTime = 60;
	DWORD maxRacingTime = 75;		// maximal allowed driving time in minutes
	
	std::string serverWelcomeMessage;
	void WriteSrvWelcomMsg(const CString driverName, bool longMsg=true);
	CString srvMessageLine[5];

	UINT srvMsgInt = 0;
	//CTimeSpan srvMsgInt (0, 0, 0, srvMsgInt);
	//UINT lastListBoxItemCnt = 0;

	rF2Chat chatWnd;

	//TCP-IP Socket Connection
	C_Log_Writer			m_C_Log_Writer;
	C_TCP_Server_Manager	*mp_C_TCP_Server_Manager = NULL;
	C_TCP_Server_Sender		*mp_C_TCP_Server_Sender = NULL;
	C_TCP_TD_SEND_STATUS	m_send_data_status;					// Status returned when sending data to the client.
	CString					sz_TCP_Adress;
	UINT					ui_TCP_Port;


	//Log File, Debugging
	std::ofstream logFileStewards;
	time_t now;

};

#endif // _rF2RaceControl_Main_H

