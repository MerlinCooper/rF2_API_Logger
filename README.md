# rF2_API_Logger

PURPOSE:
- Plugin to log calls of rFactor2 API methodes in the right order. Generates a tabulator separated list which can be further analysed in e.g a spreadsheet program.  

REMARKS
- Contains Visual Studio 2013 project file to compile a debug, release version for Win32 and x64 bit architecture.

- Recommendation: Change linker output file to the appropriate rFactor2 plugin directory.  

- IDE Debugger can be used by either starting the rFactor2 client or dedicated server. To allow sufficient time to 
  attach the debugger to the process ('rFactor.exe' or 'rFactor2 Dedicated.exe') in the debug version a modal dialog 
  is shown at when the plugin is loaded by rFactor. 

- The filename of the logfile is fixed (rF2_API_Logger.txt) and located in the rFactor2 main directory. 

- Logfile is being opened at rFactor startup and truncated to avoid to large files. 

- For every API function call the following information is written to the file. 
  runtime in µs     ... time after the logging has started
  base class        ... class where the virtual function is defined.  
  methode           ... name of the function called including return value and parameters
  remark            ... mostly used to dokument the returned value. 
  




