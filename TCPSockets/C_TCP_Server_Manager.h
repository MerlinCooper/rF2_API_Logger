//    File name: C_TCP_Server_Manager


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



/* *****************************************************************
   Purpose:  This manages the server side of a TCP connection with
   a client.
   It initializes the sockets and begins listening for the client.
   When the client solicits a connection it accepts that request and
   creates an object from class C_TCP_Server_Sender.
   That object communicates with the client.

   The app (main application) calls this object' method
   Get_Sender_Pointer()
   The the client has connected and the sender has been created, the
   pointer to the sender will be returned.  Until then the return value
   is NULL.
   After returning the pointer to the caller, the pointer is set to NULL.
   Nothing further is done.

   If the Sender exits, the application will again begin calling 
   Get_Sender_Pointer()
   Those calls will prompt this object to again listen for a client.

   ***************************************************************** */

#ifndef _C_TCP_SERVER_MANAGER
#define _C_TCP_SERVER_MANAGER

#include "C_Log_Writer.h"
#include "C_TCP_Server_Sender.h"
#include "C_TCP_Format_WSA_Text.h"


class C_TCP_Server_Manager : public CAsyncSocket
{

public:

   // C_Log_Writer is a logging utility.
   // new_port is the TCP/IP port number that will be "listened" on.
   // The original application was required to select a particlar NIC so
   // the IP address is a construction argument.
C_TCP_Server_Manager(  C_Log_Writer        *logger_pointer   = NULL, 
                       UINT                 new_port_number  = C_TCP_DEFAULT_SOCKET_PORT,
                       const char          *new_ip_address   = &C_TCP_DEFAULT_IP_ADDRESS[0] );

~C_TCP_Server_Manager( void );

C_TCP_Server_Sender * Get_Sender_Pointer(); // Returns pointer to sender or NULL

bool Initialize_Socket();
bool Class_Listen();
bool Class_Accept();
void Class_Close( );

   // Provides owner the ability to control the amount of data being logged.
   // The default is to log only status changing events. 
void Set_Value_Logging_State( bool new_value);
bool Get_Value_Logging_State();
void Get_Manager_State( char *manager_state, size_t max_length );
void Get_Socket_Name( CString& socket_address, UINT& socket_port );

void OnAccept(        int nErrorCode );
void OnConnect(       int nErrorCode );
void OnClose(         int nErrorCode );
void OnOutOfBandData( int nErrorCode );
void OnReceive(       int nErrorCode );
void OnSend(          int nErrorCode );

void Get_All_On_Call_Counts( unsigned int *on_connect,
                             unsigned int *on_receive,
                             unsigned int *on_close,
                             unsigned int *on_accept,
                             unsigned int *on_out_of_band,
                             unsigned int *on_send );

private:

   C_Log_Writer           *mp_C_Log_Writer;
   C_TCP_Server_Sender    *mp_C_TCP_Server_Sender;
   C_TCP_Format_WSA_Text   m_C_TCP_Format_WSA_Text;

  C_TCP_TD_MANAGER_STATE m_tcp_state;

   bool m_logging_state;
   bool m_method_status;
   BOOL m_winsock_status;

   unsigned int m_port_number;
   char         m_tcp_address[ C_TCP_MAX_IP_ADDRESS_LENGTH ];
   unsigned int m_wsa_error;
   unsigned int m_tcp_status_tag;

   char         m_wsa_text[    MAX_WSA_ERROR_TEXT ];
   char         m_text_to_log[ MAX_LOG_STRING_LENGTH ];

   unsigned int m_on_accept_count;
   unsigned int m_on_close_count;
   unsigned int m_on_connect_count;
   unsigned int m_on_out_of_band_count;
   unsigned int m_on_receive_count;
   unsigned int m_on_send_count;
};

#endif