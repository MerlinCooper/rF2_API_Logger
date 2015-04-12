// file name: C_TCP_Server_Manager.cpp

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


//#include "StdAfx.h"
#include "afxsock.h"
#include "C_TCP_Server_Manager.h"

/* ***************************************************************************
   Caller provides port number and pointer to the logging class
   Error logging is performed in the called methods.
   On startup this class needs:
   Pointer to the logging utility, file already opened.
   The port number to listen on.
   The IP address.  In my system there are multiple NICs and one is dedicated
   to the expected high traffic volume for telemetry data.
*************************************************************************** */

C_TCP_Server_Manager::C_TCP_Server_Manager( C_Log_Writer *logger_pointer, 
                                            UINT          new_port_number,
                                            const char   *new_ip_address )
{
   bool initialize_status = false;
   bool listen_status     = false;

   mp_C_TCP_Server_Sender = NULL;

   // Base port number for lADS is 49000. Each stream of data gets its own port
   // number ranging from 1 to 12. Add the stream number to the base port number
   // for the final port number.
   m_port_number        = new_port_number;
   mp_C_Log_Writer      = logger_pointer;
   m_logging_state      = false;
   m_tcp_state          = NOT_INITIALIZED;
   m_wsa_error          = 0;
   m_method_status      = false;
   m_winsock_status     = FALSE;
   m_wsa_error          = 0;
   m_tcp_status_tag     = 0;



   m_on_accept_count      = 0;
   m_on_close_count       = 0;
   m_on_connect_count     = 0;
   m_on_out_of_band_count = 0;
   m_on_receive_count     = 0;
   m_on_send_count        = 0;

   strcpy_s( m_tcp_address, C_TCP_MAX_IP_ADDRESS_LENGTH, new_ip_address );

      // Initialize and begin listening for the client to connect (lADS Server)
      // Remember, error logging is done within the method.
   initialize_status = Initialize_Socket();
   m_wsa_error       = WSAGetLastError( );


   if( initialize_status)
   {
      m_tcp_state = INITIALIZED_NOT_LISTENING;
      listen_status = Class_Listen( );
      if( listen_status)
      {
         m_tcp_state = LISTENING_FOR_CLIENT;
      }
   }
}

/* ***************************************************************************
   ****************************************************************************/

C_TCP_Server_Manager::~C_TCP_Server_Manager( void )
{
   if( mp_C_TCP_Server_Sender != NULL && (unsigned int) mp_C_TCP_Server_Sender != 0xCCCCCCCC )
   {
      mp_C_TCP_Server_Sender->Close( );
      delete mp_C_TCP_Server_Sender;
   }

  sprintf_s(
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Manager::~C_TCP_Server_Manager( )       Destructor called" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
}

/* ***************************************************************************
   ****************************************************************************/
/*
   The main app creates this object and polls it waiting for a pointer
   to the sender object.  This polling method does a few checks and
   can recover from errors.
*/

C_TCP_Server_Sender * C_TCP_Server_Manager::Get_Sender_Pointer()
{
   bool status = false;
   C_TCP_Server_Sender *return_ptr = NULL;

   switch( m_tcp_state )
   {

   case NOT_INITIALIZED:
      { 
         status = Initialize_Socket();


         if( status)
         {
            m_tcp_state = INITIALIZED_NOT_LISTENING;
            status = Class_Listen( );
            if( status)
            {
               m_tcp_state = LISTENING_FOR_CLIENT;
            }
         }
         break;
      }

   case INITIALIZED_NOT_LISTENING:
      { 
         status = Class_Listen( );
         if( status)
         {
            m_tcp_state = LISTENING_FOR_CLIENT;
         }

         break;
      }

   case LISTENING_FOR_CLIENT:
      { 
         if( mp_C_TCP_Server_Sender != NULL )
         {
            return_ptr = mp_C_TCP_Server_Sender;
            mp_C_TCP_Server_Sender = NULL;
         }
         break;
      }

   case CLOSING:
      { 
         break;  // do nothing.
      }

   default:
      {
      }
   }

   return return_ptr;
}

/* ***************************************************************************
   ****************************************************************************/
bool C_TCP_Server_Manager::Initialize_Socket()
{
   bool initialize_status = false;
   bool create_status     = false;

   m_winsock_status = AfxSocketInit( );
   m_wsa_error = WSAGetLastError( );

   m_winsock_status == 0 ? initialize_status = false: initialize_status = true;
   if( m_wsa_error != 0 )  initialize_status = false; 

   if( initialize_status )
   {

         // This is the server, when IP address is not supplied, use the server's
         // address.  If there are multiple NICs, OS picks one.  If you want to
         // pick a NIC, replace the NULL with its IP address.
      m_winsock_status = CAsyncSocket::Create( 
         m_port_number,
         SOCK_STREAM,
         FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,
         NULL  ); 

      m_wsa_error = WSAGetLastError( );
      m_winsock_status == 0 ? create_status = false: create_status = true;
      if( m_wsa_error != 0 )  create_status = false; 
   }

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );
   sprintf_s(
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Manager::Class_lnitialize( ) AfxSocketInit %s CAsyncSocket::Create( ) %s\n"
      "               port %d, WSA error code = %d %s",
      initialize_status == true ? "Sucess" : "Failed",
      create_status     == true ? "Sucess" : "Failed",
      m_port_number,
      m_wsa_error,
      m_wsa_text );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );

   return( initialize_status && create_status );
}

/* *******************************************************************
   ******************************************************************* */
bool C_TCP_Server_Manager::Class_Listen( )
{
   bool status = false;

   m_winsock_status  = CAsyncSocket::Listen( );
   m_wsa_error       = WSAGetLastError( );

   m_winsock_status == 0 ? status = false: status = true;
   if( m_wsa_error  != 0 ) status = false;

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );
   sprintf_s(
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Manager::Class_Listen()                 %s, WSA error code = %d %s",
      status == true ? "Success" : "Failed",
      m_wsa_error,
      m_wsa_text );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );

   return status;
}

/* ***************************************************************************
   *************************************************************************** */

bool C_TCP_Server_Manager::Class_Accept()
{
   bool new_status    = false;
   bool accept_status = false;

   sprintf_s(
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Manager::Class_Accept()                 new the sender next"  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );

   mp_C_TCP_Server_Sender = new C_TCP_Server_Sender( mp_C_Log_Writer );
   m_wsa_error = WSAGetLastError( );

   if( m_wsa_error == 0 &&  mp_C_TCP_Server_Sender != NULL ) new_status = true;

   if( new_status )
   {
      m_winsock_status = CAsyncSocket::Accept( *mp_C_TCP_Server_Sender );
      m_wsa_error = WSAGetLastError( );

      m_winsock_status == 0 ? accept_status = false : accept_status = true;
      if( m_wsa_error != 0 )  accept_status = false;
   }

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );
   sprintf_s(
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Manager::Class_Accept()                 new operation: %s Accept(): %s WSA error code = %d %s",
      new_status == true ?    "Success" : "Failed",
      accept_status == true ? "Success" : "Failed",
      m_wsa_error,
      m_wsa_text );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );

   return( new_status && accept_status );
} // end of: Class_Accept()



/* ***************************************************************************
Purpose: Allow the main dialog to set and get the logging state.
*************************************************************************** */

void C_TCP_Server_Manager::Set_Value_Logging_State( bool new_value)
{
m_logging_state = new_value;
}

/* ***************************************************************************
*************************************************************************** */

bool C_TCP_Server_Manager::Get_Value_Logging_State( )
{
return m_logging_state;
}


/* ***************************************************************************
   The main app can get the current state and display it in its dialog box
*************************************************************************** */

void C_TCP_Server_Manager::Get_Manager_State( char *manager_state, size_t max_length )
{
   if( max_length <= C_TCP_MAX_ENUM_TEXT_LENGTH )
   {
      char error_message[ 150 ];
      sprintf_s(  error_message, 
                  MAX_LOG_STRING_LENGTH, 
                  "C_TCP_Server_Manager::Get_Manager_State() reports software \nerror string argument too small: %d ",
                  max_length  );
      AfxMessageBox( error_message  );
      return; 
   }
   else
   {
      switch( m_tcp_state )
      {
      case NOT_INITIALIZED :
         {
            strcpy_s( manager_state, C_TCP_MAX_ENUM_TEXT_LENGTH,  "Not Initialized" );
            break;
         }

      case INITIALIZED_NOT_LISTENING:
         {
            strcpy_s( manager_state, C_TCP_MAX_ENUM_TEXT_LENGTH,  "Initialized Not Listening" );
            break;
         }

      case LISTENING_FOR_CLIENT:
         {
            strcpy_s( manager_state, C_TCP_MAX_ENUM_TEXT_LENGTH,  "Listening for client" );
            break;
         }

      case CLOSING:
         {
            strcpy_s( manager_state, C_TCP_MAX_ENUM_TEXT_LENGTH,  "Closing" );
            break;
         }

      default:
         {
            strcpy_s( manager_state, C_TCP_MAX_ENUM_TEXT_LENGTH,  "Invalid, software error" );
            break;
         }
      } // end of switch
   } //end of if / else
} // end of: C_TCP_Server_Manager


/* ***************************************************************************
   *************************************************************************** */

void C_TCP_Server_Manager::Class_Close()
{

   if( mp_C_TCP_Server_Sender != NULL)
   {
      delete mp_C_TCP_Server_Sender;
      mp_C_TCP_Server_Sender = NULL;
   }

   CAsyncSocket::Close( );
   m_wsa_error = WSAGetLastError( );

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );
   sprintf_s(
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Manager::Class_Close(), closed sender, closed socket, m_wsa_error %d  %s.",
      m_wsa_error,
      m_wsa_text );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
}

// end of: C_TCP_Server_Manager

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::OnAccept( int nErrorCode )
{
   m_on_accept_count ++;

   if( nErrorCode != 0 )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( nErrorCode, m_wsa_text );
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Manager::OnAccept() Error: called with non zero argument = %d  %s.",
         m_wsa_error,
         m_wsa_text );
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }

      // This method is called by Windows.  The status is not returned and all
      // errors within Class_Accept are logged there.  Disregard the return value.
   Class_Accept();
 
} // end of: OnAccept()
/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::OnConnect( int nErrorCode )
{
   m_on_connect_count ++;
}


/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::OnClose( int nErrorCode )
{
   m_on_close_count  ++;
}

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::OnOutOfBandData( int nErrorCode )
{
   m_on_out_of_band_count ++;
}

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::OnReceive( int nErrorCode )
{
   m_on_receive_count ++;
}

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::OnSend( int nErrorCode )
{
   m_on_send_count ++;
}


/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::Get_All_On_Call_Counts( unsigned int *on_connect,
                                                   unsigned int *on_receive,
                                                   unsigned int *on_close,
                                                   unsigned int *on_accept,
                                                   unsigned int *on_out_of_band,
                                                   unsigned int *on_send )
{
    *on_connect      = m_on_connect_count;
    *on_receive      = m_on_receive_count;
    *on_close        = m_on_close_count;
    *on_accept       = m_on_accept_count;
    *on_out_of_band  = m_on_out_of_band_count;
    *on_send         = m_on_send_count;
}

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Manager::Get_Socket_Name( CString& socket_address, UINT& socket_port )
{
   BOOL get_status = 0;
   get_status = GetSockName( socket_address, socket_port );
}
