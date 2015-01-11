#include "APIParser.hpp"
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono> 
#include <Windows.h>
 

// GLOBALS
const std::string version_number("API_PlugIn_Parser 0.1");


// plugin information

extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()						{ return(version_number.c_str()); }

extern "C" __declspec( dllexport )
	PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }

extern "C" __declspec( dllexport )
	int __cdecl GetPluginVersion()                         { return( 6 ); } // InternalsPluginV06 functionality

extern "C" __declspec( dllexport )
	PluginObject * __cdecl CreatePluginObject()            { return( (PluginObject *) new APIParser ); }

extern "C" __declspec( dllexport )
	void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( (APIParser*) obj ); }

//namespaces to be used
using namespace std;

inline std::string APIParser::timeAsString(){
	ostringstream oss;
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	oss << std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count();
	return oss.str();
}

APIParser::APIParser() {
#ifdef _DEBUG
	MessageBox(NULL, version_number.c_str(), "API Parser PlugIn loaded", MB_OK);
#endif
	logFile.open("APIParser.log", ios_base::out | ios_base::trunc );
	logFile << "Runtime [µs]" << "\t" << "Base Class" << "\t" << "Methode" << "\t" << "Remark" << "\n";
	logFile << timeAsString() << "\t" << "n/a" << "\t" << "APIParser::APIParser()" << "\t" << "Main Class Constructor called" << "\n";
	logFile.flush();
}

APIParser::~APIParser() {
	logFile << timeAsString() << "\t" << "n/a" << "\t" << "APIParser::~APIParser()" << "\t" << "Main Class Destructor called" << "\n";
	logFile.close();
}


/*APIParser::~InternalsPlugin() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "~InternalsPlugin()" << "\t" << "virtual destructor of base class" << "\n";
}*/

void APIParser:: Startup(long version) {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void Startup(long version)" << "\t" << "" << "\n";
}

void APIParser:: Shutdown() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void Shutdown()" << "\t" << "" << "\n";
}

void APIParser:: Load() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void Load()" << "\t" << "" << "\n";
}

void APIParser:: Unload() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void Unload()" << "\t" << "" << "\n";
}

void APIParser:: StartSession() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void StartSession()" << "\t" << "" << "\n";
}

void APIParser:: EndSession() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void EndSession()" << "\t" << "" << "\n";
}

void APIParser:: EnterRealtime() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void EnterRealtime()" << "\t" << "" << "\n";
}

void APIParser:: ExitRealtime() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void ExitRealtime()" << "\t" << "" << "\n";
}

bool APIParser:: WantsScoringUpdates() { 
	bool retVal = true;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "bool WantsScoringUpdates()" << "\t" << "returned: " << retVal << "\n";
	return retVal; 
}

void APIParser:: UpdateScoring(const ScoringInfoV01 &info) {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void UpdateScoring(const ScoringInfoV01 &info)" << "\t" << "" << "\n";
}

long APIParser::WantsTelemetryUpdates() {
	long retVal = 1L;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "long WantsTelemetryUpdates()" << "\t" << "returned: " << retVal << "\n";
	return retVal;
}

void APIParser:: UpdateTelemetry(const TelemInfoV01 &info) {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void UpdateTelemetry(const TelemInfoV01 &info)" << "\t" << "" << "\n";
}

bool APIParser:: WantsGraphicsUpdates() {
	bool retVal = true;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "bool WantsGraphicsUpdates()" << "\t" << "returned: " << retVal << "\n";
	return retVal;
}

void APIParser:: UpdateGraphics(const GraphicsInfoV01 &info) {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void UpdateGraphics(const GraphicsInfoV01 &info)" << "\t" << "" << "\n";
}

bool APIParser:: RequestCommentary(CommentaryRequestInfoV01 &info) {
	bool retVal = true;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "bool RequestCommentary(CommentaryRequestInfoV01 &info)" << "\t" << "returned: " << retVal << "\n";
	return retVal;
}

bool APIParser:: HasHardwareInputs() {
	bool retVal = false;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "bool HasHardwareInputs()" << "\t" << "returned: " << retVal << "\n";
	return retVal;
}

void APIParser:: UpdateHardware(const double fDT) {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void UpdateHardware(const double fDT)" << "\t" << "" << "\n";
}

void APIParser:: EnableHardware() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void EnableHardware()" << "\t" << "" << "\n";
}

void APIParser:: DisableHardware() {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void DisableHardware()" << "\t" << "" << "\n";
}

bool APIParser:: CheckHWControl(const char * const controlName, double &fRetVal) {
	bool retVal = true;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "bool CheckHWControl(const char * const controlName, double &fRetVal) {bool APIParser:: HasHardwareInputs()" << "\t" << "returned: " << retVal << "\n";
	return retVal;
}

bool APIParser:: ForceFeedback(double &forceValue) {
	bool retVal = true;
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "bool ForceFeedback(double &forceValue)" << "\t" << "returned: " << retVal << "\n";
	return retVal;
}

void APIParser:: Error(const char * const msg) {
	logFile << timeAsString() << "\t" << "InternalsPlugin" << "\t" << "void Error(const char * const msg)" << "\t" << "" << "\n";
}


void APIParser::SetPhysicsOptions(PhysicsOptionsV01 &options) {
	logFile << timeAsString() << "\t" << "InternalsPluginV02" << "\t" << "void SetPhysicsOptions(PhysicsOptionsV01 &options)" << "\t" << "" << "\n";
}

unsigned char APIParser::WantsToViewVehicle(CameraControlInfoV01 &camControl) { 
	unsigned char retVal = '3'; // return values: 0=do nothing, 1=set ID and camera type, 2=replay controls, 3=both
	logFile << timeAsString() << "\t" << "InternalsPluginV03" << "\t" << "unsigned char WantsToViewVehicle(CameraControlInfoV01 &camControl)" << "\t" << "returned: " << retVal << "\n";
	return retVal; 
} 


void APIParser::UpdateGraphics(const GraphicsInfoV02 &info) {
	logFile << timeAsString() << "\t" << "InternalsPluginV03" << "\t" << "void UpdateGraphics(const GraphicsInfoV02 &info)" << "\t" << "" << "\n";
} 

bool APIParser::WantsToDisplayMessage(MessageInfoV01 &msgInfo) { 
	bool retVal = false;
	logFile << timeAsString() << "\t" << "InternalsPluginV03" << "\t" << "bool WantsToDisplayMessage(MessageInfoV01 &msgInfo)" << "\t" << "returned: " << retVal << "\n";
	return retVal; 
} 

void APIParser::SetEnvironment(const EnvironmentInfoV01 &info){
	logFile << timeAsString() << "\t" << "InternalsPluginV04" << "\t" << "void SetEnvironment(const EnvironmentInfoV01 &info)" << "\t" << "" << "\n";
} 

 void APIParser:: InitScreen(const ScreenInfoV01 &info){
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void InitScreen(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 } 

 void APIParser:: UninitScreen(const ScreenInfoV01 &info){ 
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void UninitScreen(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 } 

 void APIParser:: DeactivateScreen(const ScreenInfoV01 &info){
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << " void DeactivateScreen(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 } 

 void APIParser:: ReactivateScreen(const ScreenInfoV01 &info){
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void ReactivateScreen(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 }

 void APIParser:: RenderScreenBeforeOverlays(const ScreenInfoV01 &info) {
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void RenderScreenBeforeOverlays(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 } 

 void APIParser:: RenderScreenAfterOverlays(const ScreenInfoV01 &info) {
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void RenderScreenAfterOverlays(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 } 

 void APIParser:: PreReset(const ScreenInfoV01 &info) {
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void PreReset(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 } 

 void APIParser:: PostReset(const ScreenInfoV01 &info) {
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "void PostReset(const ScreenInfoV01 &info)" << "\t" << "" << "\n";
 }

 bool APIParser::InitCustomControl(CustomControlInfoV01 &info) { 
	 bool retVal = false;
	 logFile << timeAsString() << "\t" << "InternalsPluginV05" << "\t" << "bool InitCustomControl(CustomControlInfoV01 &info)" << "\t" << "returned: " << retVal << "\n";
	 return retVal; 
 } 

  bool APIParser::WantsWeatherAccess() {
	  bool retVal = true;
	  logFile << timeAsString() << "\t" << "InternalsPluginV06" << "\t" << "bool WantsWeatherAccess()" << "\t" << "returned: " << retVal << "\n";
	  return retVal; 
  } 

  bool APIParser::AccessWeather(double trackNodeSize, WeatherControlInfoV01 &info) { 
	  bool retVal = true;
	  logFile << timeAsString() << "\t" << "InternalsPluginV06" << "\t" << "bool AccessWeather(double trackNodeSize, WeatherControlInfoV01 &info)" << "\t" << "returned: " << retVal << "\n";
	  return retVal;
  }

  void APIParser::ThreadStarted(long type){
	  logFile << timeAsString() << "\t" << "InternalsPluginV06" << "\t" << "void ThreadStarted(long type)" << "\t" << "" << "\n";
  } 

  void APIParser::ThreadStopping(long type) {
	  logFile << timeAsString() << "\t" << "InternalsPluginV06" << "\t" << "void ThreadStopping(long type)" << "\t" << "" << "\n";
  }