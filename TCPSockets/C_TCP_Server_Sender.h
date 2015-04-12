//    File C TCP Server Send.h

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


/* **************************************************************************

   This will perform the activities of sending to the client.
   It is created by the manager. 

   MAJOR CAVEAT
   This was created to send data to a particular data display system.  
   Immediatly on startup it always sends two one byte messages.  They specify
   the Endian mode and the type of payload packets that will be sent.
   After that it only sends data.

   ************************************************************************* */

#pragma once
#include "afxsock.h"
#include "C_TCP_Constants.h"
#include "C_LOG_Writer.h"
#include "C_TCP_IADS_Typedefs.h"
#include "C_TCP_Format_WSA_Text.h"
#include "C_TCP_Consts_and_Enums.h"


   // The end of this #ifdef is at the very bottom
   // Visual Studio put this in, not me
#if !defined( AFX_C_TCP_Server_Sender_H_166D4120_2F94_4231_AE60_7C719E3EC05C_INCLUDED_)
   #define AFX_C_TCP_Server_Sender_H_166D4120_2F94_4231_AE60_7C719E3EC05C_INCLUDED_

 
   // The length of the text used when logging the value of the 
   // of sender state
const unsigned int MAX_TCP_STATE_TEXT = 32;

/////////////////////////////////////////////////////////////////////////////

class C_TCP_Server_Sender: public CAsyncSocket
{


public:
   // Constructor, needs a pointer to the logging utility.
C_TCP_Server_Sender( C_Log_Writer *ptr_C_Log_Writer );

virtual ~C_TCP_Server_Sender( );

   // Send the data to the client. Note that flags is optional
C_TCP_TD_SEND_STATUS Class_Send( const void *data_to_send, int size, int flags = 0);

int Class_Receive( char *receive_buffer, unsigned int max_requested, int flags = 0 );

void Reset_Counters();

private:

   // send a one byte message to client specifying little or big endian
   // See constants above
bool Class_Send_Endian();

   // send a one byte message to client specifying the type of payload packet  
   // See constants above
bool Class_Send_Frame_Type( DWORD payload_type = C_IADS_TAG_SIZE_VALUE );

   // Post a TCP/IP read.  Enables detecting when client closes the connection
void Class_Read( );

   // Translates the state of this class to text so the log file does not show 
   // an opaque integer for the state.
void Translate_TCP_STATE( C_TCP_TD_TCP_STATE input, char *output_text );

   // These methods buffer up data when the socket cannot accept a call to Send( )
   // and sends that data when allowed.
void Buffer_A_Message( const void *data_to_send, unsigned __int16 length );

   // Sends out the buffered data
void Send_Buffered_Data( );

// Overrides
public:
void SetParentDlg(CDialog *pDlg);

// Class Wizard generated virtual function overrides
// {{AFX_VIRTUAL(C_TCP_Server_Sender)

public:



virtual void OnClose( int nErrorCode );
virtual void OnSend(  int nErrorCode );
virtual void OnAccept( int nErrorCode );
virtual void OnConnect( int nErrorCode );
virtual void OnOutOfBandData( int nErrorCode );
virtual void OnReceive( int nErrorCode );

void Get_All_On_Call_Counts( unsigned int *on_connect,
                             unsigned int *on_receive,
                             unsigned int *on_close,
                             unsigned int *on_accept,
                             unsigned int *on_out_of_band,
                             unsigned int *on_send );

unsigned int Get_Buffer_Fill_Level();
unsigned int Get_Blocked_Count();
unsigned int Get_Buffer_Fill_Max_Count();
unsigned int Get_Lost_Message_Count();

//} }AFX_VIRTUAL
// Generated message map functions
// {{AFX_MSG(C_TCP_Server_Sender)
// NOTE - the ClassWizard will add and remove member functions here.
//} }AFX_MSG

// Implementation
protected:
private:


   // Translates the most likely WSA error codes to text for logging
C_TCP_Format_WSA_Text m_C_TCP_Format_WSA_Text;

   // Pointer to the logging utility so this class can log status info.
C_Log_Writer *mp_C_Log_Writer;

   // TCP/IP errors always captured here
int m_wsa_error;

char m_read_buffer[ C_TCP_SENDER_READ_BUFFER];

  // state of the TCP communications with the client
C_TCP_TD_TCP_STATE   m_tcp_state;

  // status from the Send request from the owner.  Succeeded, Failed, time to exit
C_TCP_TD_SEND_STATUS m_send_status;

   // These are for logging errors and status
unsigned int   m_length_of_log_message;
char           m_text_to_log[ MAX_LOG_STRING_LENGTH ];
char           m_wsa_text[ MAX_WSA_ERROR_TEXT ];
char           m_tcp_state_text[ MAX_TCP_STATE_TEXT ];

/* 
The next few declarations are for buffering data. The original application can send data
very fast and often solicits the error code WSAEWOULDBLOCK. That means that if this had been
blocking code, it would block. As this is asynchronous, and the code does not (cannot)
wait until it would not block, the data has not been sent.
That means that the data must be saved until it can be sent.
When the data can be sent, method OnSend( ) is called by Windows. That method will
send the buffered data.
SEND_BUFFER_DEPTH is declared above and defines the number of buffers that can be held

m_buffer_insert specifies where in the buffer data will be placed when it is received
but cannot be sent.

m_buffer_remove specifies where in the buffer data will be extracted from and sent
when it is okay to send again

When m_buffer_insert == m_buffer_remove there are two possible meanings:

When m_buffer_empty  == true : the buffer is empty
When m_buffer_empty  == false: the buffer is full.

   *************************************************************************** */

   // How many entries are in the buffer
unsigned int m_buffer_fill_depth;

 
 // control data logging
bool  m_log_would_block;
bool  m_log_buffer_send;
bool  m_log_buffered_data;


bool  m_buffer_empty;
bool  m_buffer_full;
bool  m_sending_allowed;

unsigned int m_buffer_fill_max_count;
unsigned int m_lost_message_count;

   // This is the index value where the next payload packet   
   // is to be inserted. It is always incremented after  
   // each insertion. If, after the insertion, it is equal to 
   // the remove index, then the buffer is full, m_buffer_full is set true.
unsigned int m_buffer_insert_index;

   // This is the index value where the next payload packet is to be 
   // removed from.  When removing data, keep removing until this is 
   // equal to the insert index.
   // When the two index values are equal, declare buffer empty.
unsigned int m_buffer_remove_index;

   // declare a union that can be accessed as a structure.
   // This enables the code to detect the sequence number.  It was vital
   // during development and testing.
union TD_SEND_BUFFER_DATA
{
   char char_format[ C_IADS_SIZE_TD_MESSAGE_PACKET ];  // use to send data
   C_IADS_TD_FIXED_PARAMETER_PACKET iads_format;       // use to look at fields within
                                                // the payload packet
};

// Put that union'ed buffer into a structure that can also
// hold the length.  These structurs are put in the buffer for later sends.

struct TD_MESSAGE_BUFFER
{
TD_SEND_BUFFER_DATA data_to_send;
int length_to_send;
};

   // Declare an array instance of that structure.
   // This is the final declaration where the buffered data is stored.
TD_MESSAGE_BUFFER m_message_buffer[ C_TCP_SEND_BUFFER_DEPTH ];

   // Declare a pointer to that structure.
TD_SEND_BUFFER_DATA *mp_message_buffer;

   // number of times OnSend() has been called.
int m_on_send_count;
int m_on_close_count;
int m_on_accept_count;
int m_on_connect_count;
int m_on_out_of_band_count;
int m_on_receive_count;
unsigned int m_sending_blocked_count;


};


/////////////////////////////////////////////////////////////////////////////
//{ {AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif //  !defined(AFX_C_TCP_Server_Sender_H_166D4120_2F94_4231_AE60_7C719E3EC05C_INCLUDED_)

