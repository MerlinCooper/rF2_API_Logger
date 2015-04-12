// File name: C_TCP_Server_Sender.Cpp

/* ******************************************************************
    (C) Copyright 2012/2013 Bryan Kelly
    This file is part of the software utility named:
    Bryan Kelly's TCP/IP Starter for Microsoft Windows.
    The short name is BK TCP/IP Starter.

    BK TCP/IP Starter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BK TCP/IP Starter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BK TCP/IP Starter.  If not, see <http://www.gnu.org/licenses/>.

    EXCEPTION to the GNU license for this application:  
    United States Government, U.S. Dod, and other U.S. government agencies
    may use this and all derived or similar utilities for non-commercial
    use without the copyright notice.
********************************************************************* */


/* ***************************************************************************
Class C_Log_Writer
file name: C_Log_Writer.h

Provides the ability to write information to a log file.
 
It opens a log file and accepts text to write to the file.
Log file name is based on a prefix provided by the owner (calling application)
followed by the time it is opened. This enables multiple applications to have 
log files open simultaneously.
To prevent excessivly large files, it opens a new file every 
LOG_FILE_RESTART_COUNT entries.

Caution:  Take care to delete log files.
*********************************************************************** */

#pragma once

#include <afxwin.h>

const char DEFAULT_DlRECTORY[ 16] = "C:\\TEMP\\";

   // Multiple application can use this utility simultaneously.  Provide a prefix
   // to identify each log.  This is the default prefix.  It is up to the user
   // to keep the names unique.
const char DEFAULT_FILENAME_PREFIX[ 8] = "xx";

const char CLOSE_FILE_NOTICE[] = "Application is closing the log file";
const int  LENGTH_CLOSE_NOTICE = sizeof( CLOSE_FILE_NOTICE );

   // Prevent the log file from growing too large.  After this many 
   // entries have been posted, automatically start a new log file.
const unsigned int LOG_FILE_RESTART_COUNT = 20000;

   // Set some limit constants.  The names should be self explanatory.
const unsigned int MAX_DATE_TIME_LENGTH      = 16;
const unsigned int MAX_STRING_NAME_LENGTH    = 32;
const unsigned int MAX_DIRECTORY_NAME_LENGTH = 32;
const unsigned int MAX_FILE_NAME_LENGTH      = MAX_STRING_NAME_LENGTH + MAX_DIRECTORY_NAME_LENGTH;
const unsigned int MAX_LOG_STRING_LENGTH     = 300;

   // Just in case the user want an easy to access constant providing the longest
   // allowed log entry.
const unsigned int MAX_USER_LOG_LENGTH       = MAX_LOG_STRING_LENGTH - MAX_DATE_TIME_LENGTH;

class C_Log_Writer
{
public:
   C_Log_Writer(void);
  ~C_Log_Writer(void);

      // A complete startup constructor.
   C_Log_Writer( const char *new_directory_name, const char *new_name_prefix );

   void Set_Prefix_String(    const char *new_prefix_string    );
   void Set_Directory_String( const char *new_directory_string );

   void Open_Log_File();

      // Call anytime desired.  Automatically called after LOG_FILE_RESTART_COUNT log writes.
   void Re_Open_Log_File();

   void Close_Log_File();
   void Enable_Log_File_Writes( bool enable );  // suspend or enables writes without closing.
   bool Get_Enabled_Status();

      // The primary user interface
   bool Write_Log_File_Entry( const char *data_to_log );

private:

   void Build_Log_File_Name();

     // Count the number of log entries made. Open a new log file every N entries
     // to keep the size managable.
   unsigned int m_log_entry_count;

     // Enable or disable all data logging
   bool m_log_data_enabled;

   HANDLE m_log_file_handle;
   SYSTEMTIME m_time_of_day;
   char m_prefix_string[    MAX_STRING_NAME_LENGTH ];
   char m_directory_string[ MAX_DIRECTORY_NAME_LENGTH ];
   char m_file_name[        MAX_FILE_NAME_LENGTH ];
   char m_log_message[      MAX_LOG_STRING_LENGTH ]; 

   int   m_write_length;
   DWORD m_bytes_written;
   DWORD m_hour;
   DWORD m_minute;
   DWORD m_second;
};