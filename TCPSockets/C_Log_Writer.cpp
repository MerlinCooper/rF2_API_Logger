// file name: C_Log_Writer.cpp

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

//#include "stdafx.h"
#include "C_Log_Writer.h"

   // Simple constructor
C_Log_Writer::C_Log_Writer(void)
{
   m_log_data_enabled = true;
   m_log_file_handle = NULL;
   m_log_entry_count = 0;
   m_hour   = 0;
   m_minute = 0;
   m_second = 0;
   sprintf_s( m_prefix_string, 
              MAX_STRING_NAME_LENGTH, 
              "%s",
              DEFAULT_FILENAME_PREFIX);
}

   // Comprehensive constructor
C_Log_Writer::C_Log_Writer( const char *new_directory_name = &DEFAULT_DlRECTORY[0], 
                            const char *new_name_prefix = &DEFAULT_FILENAME_PREFIX[0] )
{
   m_log_data_enabled = true;
   m_log_file_handle = NULL;
   m_log_entry_count = 0;
   m_hour   = 0;
   m_minute = 0;
   m_second = 0;
   Set_Directory_String( new_directory_name );
   Set_Prefix_String( new_name_prefix );
   Open_Log_File();
}

/*  ***************************************************************************
Destructor
*********************************************************************** */
C_Log_Writer::~C_Log_Writer(void)
{
   if( m_log_file_handle != NULL)
   {
      char close_notice[] = "~C_Log_Writer is closing the log file";
      Write_Log_File_Entry( close_notice );
      CloseHandle( m_log_file_handle);
      m_log_file_handle = NULL;
   }
}


/* ***************************************************************************
Purpose: Provides the ability for the main application to provide a 
specific prefix for easy identification of the log file.  Usefull when
multiple applications are active and write to log files.
*********************************************************************** */
void C_Log_Writer::Set_Prefix_String( const char * new_prefix_string )
{
   strcpy_s( m_prefix_string, MAX_STRING_NAME_LENGTH, new_prefix_string );
}

/* ***************************************************************************
Same for the directory name
*********************************************************************** */
void C_Log_Writer::Set_Directory_String( const char * new_directory_string )
{
   strcpy_s( m_directory_string, MAX_STRING_NAME_LENGTH, new_directory_string );
}



/*  ***************************************************************************

Provide an external ability to enable and disable logging.
*********************************************************************** */
void C_Log_Writer::Enable_Log_File_Writes( bool enable)
{
   m_log_data_enabled = enable;
}

/* ***************************************************************************

Create the log file.
*********************************************************************** */

void C_Log_Writer::Open_Log_File()
{
   const int lpSecurity_Attributes = 0;
   const int dwCreationDisposition = 0;
   const int dwFlagsAndAttributes  = 0;

   Build_Log_File_Name();
   m_log_file_handle = CreateFileA(
      m_file_name,            // 1
      FILE_WRITE_DATA,        // 2 DWORD dwDesiredAccess,
      FILE_SHARE_WRITE,       // 3 dwShareMode
      NULL,                   // 4 LPSECURITY ATTRIBUTES  lpSecurity_Attributes,
      CREATE_ALWAYS,          // 5 dwCreationDisposition,
      FILE_ATTRIBUTE_NORMAL,  // 6  dwFlagsAndAttributes,
      NULL );                 // 7 optional HANDLE hTemplate file

   if( m_log_file_handle == INVALID_HANDLE_VALUE )
   {
      DWORD last_error = GetLastError();

      sprintf_s(  m_log_message, 
                  MAX_LOG_STRING_LENGTH, 
                  "C_Log_Writer::Open_Log_File() reports error %d, \nUnable to open log file \n%s"
                  "\n see this web page: http://msdn.microsoft.com/en-us/library/ms681381(VS.85).aspx ",
                  last_error,
                  m_file_name  );
      AfxMessageBox( m_log_message  );
      m_log_data_enabled = false;
   }
   else
   {
         // This can be called as a result of Re_Open_Log_File()
         // Always put at least one entry in the log file.
         // But don't lose the user's logging state.
      bool save_m_log_data_enabled = m_log_data_enabled;

      m_log_data_enabled = true;
      char open_notice[] = "Log file opened";
      Write_Log_File_Entry( open_notice );

      m_log_data_enabled = save_m_log_data_enabled;
   }
} // end of: Open_Log_File()

/*  ***************************************************************************
Purpose: Break out the code to build the log file name.
Used only when opening a new log file.
*********************************************************************** */

void C_Log_Writer::Build_Log_File_Name()
{
   GetSystemTime( &m_time_of_day);
   // A simple check to prevent opening a new file within the second and
   // over-writing previous data.
   if( m_time_of_day.wHour   == m_hour &&
       m_time_of_day.wMinute == m_minute &&
       m_time_of_day.wSecond == m_second )
   {
      m_time_of_day.wSecond ++;
   }
   else
   {
      m_second = m_time_of_day.wSecond;
   }

   m_second = m_time_of_day.wSecond;
   m_hour   = m_time_of_day.wHour;
   m_minute = m_time_of_day.wMinute;

     // format is: directory\prefix_YYYY_MM_DD_HH_MM_SS
   sprintf_s( m_file_name,
      MAX_FILE_NAME_LENGTH,
      "%s%s_%04d_%02d_%03d-%03d-%02d-%02d.txt",
      m_directory_string,
      m_prefix_string,
      m_time_of_day.wYear,
      m_time_of_day.wMonth,
      m_time_of_day.wDay,
      m_time_of_day.wHour,
      m_time_of_day.wMinute,
      m_time_of_day.wSecond );

}  // end of: Build_Log_File_Name

/* ***************************************************************************
Owner can start a new log file.
*********************************************************************** */

void C_Log_Writer::Re_Open_Log_File()
{
   char close_notice[] = "Closing log file, opening new log file";
   Write_Log_File_Entry( close_notice );
   Close_Log_File();
   Open_Log_File();
}


/*  ***************************************************************************
This is the real worker.
Write to the log file. Preface each entry with the current time.
*********************************************************************** */

bool C_Log_Writer::Write_Log_File_Entry( const char *data_to_log )
{

   unsigned int argument_length = strlen( data_to_log );
   if( (argument_length + MAX_DATE_TIME_LENGTH) >= (MAX_LOG_STRING_LENGTH - 5 ))
   {
      return false;
   }

   if( m_log_data_enabled && m_log_file_handle != NULL)
   {
      GetSystemTime( &m_time_of_day);
      sprintf_s( m_log_message,
         MAX_LOG_STRING_LENGTH,
         "\n%02d:%02d:%02d.%03d ",
         m_time_of_day.wHour,
         m_time_of_day.wMinute,
         m_time_of_day.wSecond,
         m_time_of_day.wMilliseconds );

      strcat_s( m_log_message, MAX_LOG_STRING_LENGTH, data_to_log );
      m_write_length = strlen( m_log_message );

      WriteFile( m_log_file_handle,
         &m_log_message,
         m_write_length,
         &m_bytes_written,
         NULL );

      if( ( m_log_entry_count ++ ) > LOG_FILE_RESTART_COUNT )
      {
         m_log_entry_count = 0;
         Re_Open_Log_File();
      }
   }
   return true;
} // end of: Write_Log_File_Entry( ...)

/*  ***************************************************************************
*********************************************************************** */

void C_Log_Writer::Close_Log_File()
{
   if( m_log_file_handle != NULL)
   {
      Write_Log_File_Entry( &CLOSE_FILE_NOTICE[0] );

      CloseHandle( m_log_file_handle );
      m_log_file_handle = NULL;
   }
}

/*  ***************************************************************************
    *********************************************************************** */

bool C_Log_Writer::Get_Enabled_Status()
{
   return m_log_data_enabled;
}