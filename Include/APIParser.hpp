//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//›                                                                         ﬁ
//› Module: PlugIn to parse API Interface                                   ﬁ
//›                                                                         ﬁ
//ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ

#ifndef _APIPARSER_H
#define _APIPARSER_H 

#include "InternalsPlugin.hpp"

#include <set>
#include <vector>
#include <list>
#include <fstream>
#include <ctime>
#include <chrono> 



class APIParser : public InternalsPluginV06{
public: 
	APIParser();
	~APIParser();
protected: 
	//methodes derived from InternalsPlugIn
	//~InternalsPlugin();
	void Startup(long version);
	void Shutdown();
	void Load();
	void Unload();
	void StartSession();
	void EndSession();
	void EnterRealtime();
	void ExitRealtime();
	bool WantsScoringUpdates();
	void UpdateScoring(const ScoringInfoV01 &info);
	long WantsTelemetryUpdates();
	void UpdateTelemetry(const TelemInfoV01 &info);
	bool WantsGraphicsUpdates();
	void UpdateGraphics(const GraphicsInfoV01 &info);
	bool RequestCommentary(CommentaryRequestInfoV01 &info);
	bool HasHardwareInputs();
	void UpdateHardware(const double fDT);
	void EnableHardware();
	void DisableHardware();
	bool CheckHWControl(const char * const controlName, double &fRetVal);
	bool ForceFeedback(double &forceValue);
	void Error(const char * const msg);

	//methodes derived from InternalsPlugInV02
	void SetPhysicsOptions(PhysicsOptionsV01 &options);

	//methodes derived from InternalsPlugInV03
	unsigned char WantsToViewVehicle(CameraControlInfoV01 &camControl); 
	void UpdateGraphics(const GraphicsInfoV02 &info);
	bool WantsToDisplayMessage(MessageInfoV01 &msgInfo);

	//methodes derived from InternalsPlugInV04
	void SetEnvironment(const EnvironmentInfoV01 &info);

	//methodes derived from InternalsPlugInV05
	void InitScreen(const ScreenInfoV01 &info);
	void UninitScreen(const ScreenInfoV01 &info);
	void DeactivateScreen(const ScreenInfoV01 &info);
	void ReactivateScreen(const ScreenInfoV01 &info);
	void RenderScreenBeforeOverlays(const ScreenInfoV01 &info);
	void RenderScreenAfterOverlays(const ScreenInfoV01 &info);
	void PreReset(const ScreenInfoV01 &info);
	void PostReset(const ScreenInfoV01 &info);
	bool InitCustomControl(CustomControlInfoV01 &info);


	//methodes derived from InternalsPlugInV06
	bool WantsWeatherAccess();
	bool AccessWeather(double trackNodeSize, WeatherControlInfoV01 &info);
	void ThreadStarted(long type);
	void ThreadStopping(long type);


private:
	std::ofstream logFile;
	inline std::string timeAsString();
	const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	//static auto startTime = std::chrono::high_resolution_clock::now();		// const time_t startTime = time(0);
	//CTime sysTime;
};

#endif // _APIPARSER_H

