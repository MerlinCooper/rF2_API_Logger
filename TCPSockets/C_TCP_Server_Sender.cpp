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

/*

This class C_TCP_Server_Sender is intended to work as part of an
asynchronous project. This will perform the activities of sending and
receiving data, after the Create( ) and Listen( ) have already completed.
It is created by C_TCP_Server_Manager

Note:  This is tailored to work with IADS (Interactive Analysis and
   Display System, produced by Symvionics).  When the connection is
   made it must send two one byte messages. The first specifies 
   big or little endian, the second specifies the format of the 
   payload packets.  It is not general purpose, but not far
   from it.

   Relatively rare events are always noted in the log file.
*/
//#include "stdafx.h"
#include "C_TCP_Server_Sender.h"
#include "C_TCP_Server_Manager.h"


/* *************************************************************************
Include a pointer to the logging utility for status logging.
************************************************************************* */

C_TCP_Server_Sender::C_TCP_Server_Sender( C_Log_Writer *ptr_C_Log_Writer )
{
   mp_C_Log_Writer  = ptr_C_Log_Writer;

   // The first order of business is set the FSM state to send the Endian message to lADS.
   m_tcp_state   = SEND_ENDIAN;
   m_send_status = READY_TO_SEND;

   m_wsa_error   = 0;

   m_sending_allowed = false;   // Prohibit sending until OnSend( ) is called.
   m_log_would_block = false;   // do not log would block until requested
   m_log_buffer_send = false;   // do not log all sends until requested
   m_log_buffered_data = false;

   m_on_send_count         = 0;
   m_on_close_count        = 0;
   m_on_accept_count       = 0;
   m_on_connect_count      = 0;
   m_on_out_of_band_count  = 0;
   m_on_receive_count      = 0;
   m_sending_blocked_count = 0;
   m_buffer_fill_max_count = 0;
   m_lost_message_count    = 0;

      // See comments in dot H file about the data buffer.
   m_buffer_empty        = true;
   m_buffer_full         = false;
   m_buffer_insert_index = 0;
   m_buffer_remove_index = 0;
   m_buffer_fill_depth   = 0;


   // Only the lengths of the data being buffered need to be initialized.
   for( int i = 0; i < C_TCP_SEND_BUFFER_DEPTH; i++)
   {
      m_message_buffer[ i ].length_to_send = 0;
   }

// At this point the first call of OnSend() has not occurred, permission has
// not been granted to send.  See OnSend()

     // Log the startup event.
   sprintf_s( m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Server_Sender()                         constructor reports completion." );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
}

/* ************************************************************************
   ************************************************************************  */

C_TCP_Server_Sender::~C_TCP_Server_Sender( )
{
   CAsyncSocket::Close( );
     sprintf_s( m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Server_Sender::~C_TCP_Server_Sender( )  Destructor called" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
}

// Do not edit the following lines, which are needed by Class Wizard.
#if 0
BEGIN_MESSAGE_MAP(C_TCP_Server_Send, CAsyncSocket)
     // {{AFX_MSG_MAP(C_TCP_Server_Send)
     // }}AFX_MSG_MAP
END_MESSAGE_MAP( )
#endif // 0



/* *************************************************************************
Send the Endian state to lADS
   ************************************************************************* */

bool C_TCP_Server_Sender::Class_Send_Endian( )
{
   char endian_mode = C_TCP_LITTLE_ENDIAN;
   int character_send_count = 0;
   int buffer_length        = 1;

   character_send_count = CAsyncSocket::Send( (char*) &endian_mode, buffer_length );
   m_wsa_error = WSAGetLastError( );

   m_wsa_error == 0 ? m_sending_allowed = true : m_sending_allowed = false;
   
   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );
   sprintf_s( m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C TCP Server Send::Class_Send Endian( )       m_sending_allowed = %s, Endian mode = %s WSA error code = %d %s",
      m_sending_allowed == true ? "True" : "False",
      endian_mode == C_TCP_LITTLE_ENDIAN ? "Little Endian" : "Big Endian",
      m_wsa_error,
      m_wsa_text  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );

   return m_sending_allowed;
}


/* **********************************************************************
Purpose: Send the Frame type to lADS
   ********************************************************************** */
bool C_TCP_Server_Sender::Class_Send_Frame_Type( DWORD payload_type )
{
 
   bool status     = false;
   int  count_sent = 0;

      // Send one integer
   count_sent = CAsyncSocket::Send( (char*)&payload_type, sizeof( payload_type) );
   m_wsa_error = WSAGetLastError( );

   if( m_wsa_error != 0 )
   {
      m_sending_allowed = false;
   }

   if( count_sent == 4 && m_wsa_error == 0 )
   {
      status = true;
   }

      // This is a relatively rare event, always log it.
   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );
   sprintf_s( m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Server_Sender::Class_Send_Frame_Type( ) payload type = %s status = %s, WSA error code = %d %s ",

      payload_type == C_IADS_TAG_SIZE_VALUE ? "Tag/Size/Value" : "Tag/Tag/Value/Value",
      status == true ? "True" : "False" ,
      m_wsa_error,
      m_wsa_text );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   return status;
} // end of: Send_Frame_Type( )

/* ***************************************************************************
   Sends the data.
   When the WSA Error is WSAEWOULDBLOCK, the data was not sent.
   Therefore: buffer the data and continue buffering data until method
   CAsyncSocket::OnSend( ) is called.
*************************************************************************** */

C_TCP_TD_SEND_STATUS C_TCP_Server_Sender::Class_Send( const void *data_to_send, int length, int flags )
{
   int chars_sent = 0;
   bool status = false;

      // If we must exit, don't attempt to send, just notify the main app.
   if( m_tcp_state == SENDER_MUST_EXIT )
      return SENDER_READY_TO_EXIT;

#pragma region log_send
   if( m_log_buffer_send)
   {
      mp_message_buffer = (TD_SEND_BUFFER_DATA *) data_to_send;

      Translate_TCP_STATE( m_tcp_state, m_tcp_state_text );
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::Class_Send( ) entry line %4d sequence # %6d wsa error %5d state %5d %s m_sending_allowed %d",
         __LINE__,
         mp_message_buffer->iads_format.header.sequence_number,
         m_wsa_error,
         m_tcp_state,
         m_tcp_state_text,
         m_sending_allowed );
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
#pragma endregion

   switch( m_tcp_state)
   {

   case SEND_DATA:
      {
         // If not in a blocking state.  
         if( m_sending_allowed )
         {
            chars_sent = CAsyncSocket::Send( (const void *) data_to_send, length, flags );
            m_wsa_error = WSAGetLastError( );

            if( chars_sent == length && m_wsa_error == 0 )
            {
               m_send_status = SEND_OKAY;
            }
            else if( m_wsa_error == WSAEWOULDBLOCK )
            {
               m_sending_allowed = false;
               m_sending_blocked_count ++;
               m_send_status = SEND_BLOCKED;
            }
            else
            {
               m_send_status = SEND_FAIL;
            }
  


#pragma region write_log  
              /*
                 Four reasons to log data here
                 1.  The user requests log all sends
                 2.  The requested count was not sent, but exclude WSAEWOULDBLOCK because that causes send length of -1
                 3.  The user request logging WSAEWOULDBLOCK and it happened
                 4.  There is any wsa error code other than 0 and WSAEWOULDBLOCK.
              */
            if(  m_log_buffer_send    ||                                         // 1.
               ( chars_sent != length  && m_wsa_error != WSAEWOULDBLOCK )    ||  // 2.
               ( m_log_would_block     && m_wsa_error == WSAEWOULDBLOCK )    ||  // 3.
               ( m_wsa_error != 0      && m_wsa_error != WSAEWOULDBLOCK ) )      // 4.
            {
               // set ths pointer so the sequence number can be identified.
               mp_message_buffer = (TD_SEND_BUFFER_DATA *) data_to_send;

               m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text);
               sprintf_s( m_text_to_log,
                  MAX_LOG_STRING_LENGTH,
                  "C_TCP_Server_Sender::Class_Send( ) line %4d sequence # %6d length %5d sent %5d sending allowed %d wsa error %5d %s ",
                  __LINE__,
                  mp_message_buffer->iads_format.header.sequence_number,
                  length,
                  chars_sent,
                  m_sending_allowed,
                  m_wsa_error,
                  m_wsa_text   );
               mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
            }
#pragma endregion

         } // end of: if( m_sending_allowed)

            // No else here, m_sending_allowed can be changed by the above send operation
         if( !m_sending_allowed ) // set false with m_wsa_error == WSAEWOULDBLOCK)
         {
            // If the Send just caused a blocking state OR
            // the blocking state was already in effect THEN
            // buffer up the message.
            m_sending_allowed = false;
            Buffer_A_Message( data_to_send, length );

         }
         break;
      } // end of: case SEND_DATA:

   case SEND_ENDIAN:
      {
         status = Class_Send_Endian( ); // the method logs any errors
         if( status)
         {
            // Short cut here. If Endian is good, send mode right away.
            m_tcp_state = SEND_FRAME;
            status = Class_Send_Frame_Type( );
            if(status)
            {
               m_tcp_state = SEND_DATA;
            }
            else
            {
               // We could get the would block error code, so stay in SEND_FRAME if
               // error code is not zero.  Nothing else to do.
            }
         }
         break;
      } // end of: case SEND_ENDIAN

   case SEND_FRAME:
      {
         status = Class_Send_Frame_Type( );
         if( m_wsa_error == 0 )
         {
            m_tcp_state = SEND_DATA;
         }
         else if( m_wsa_error == WSAEWOULDBLOCK)
         {
            // do nothing, stay in this state
         }
         else
         {
            // There is an error that cannot be handled here. Close the socket
            // and return the kill me request.
            CAsyncSocket::Close( );
            m_tcp_state = SENDER_MUST_EXIT;
            return SENDER_READY_TO_EXIT;    
         }
         break;
      }

   default:
      {
         sprintf_s( m_text_to_log,
            MAX_LOG_STRING_LENGTH,
            "C_TCP_Server_Send_Class_Send( ) line %4d switch default taken, software error m_tcp_state = %d ",
            __LINE__,
            m_tcp_state );
         mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
      }

   } // end of: switch

   if( m_tcp_state == SENDER_MUST_EXIT )
      return SENDER_READY_TO_EXIT;
   else
      return m_send_status;

} // end of: Class_Send( )


/* ***************************************************************************
   Purpose: Post a read to get the OnClose( ). Allows this object to get
   the call to OnClose() when the client closes the connection.
//*************************************************************************** */

void C_TCP_Server_Sender::Class_Read( )
{
   int count = CAsyncSocket::Receive( &m_read_buffer, C_TCP_SENDER_READ_BUFFER, 0 );
   m_wsa_error = WSAGetLastError( );
}


/* ***************************************************************************
   Called by OS when client closes connection.
   *************************************************************************** */

void C_TCP_Server_Sender::OnClose(int nErrorCode)
{
   m_on_close_count ++;

   CAsyncSocket::OnClose(nErrorCode);
      // main app will be notified next time it
      // attempts to send data.
   m_tcp_state = SENDER_MUST_EXIT;  

   sprintf_s( 
      m_text_to_log,
      MAX_LOG_STRING_LENGTH,
      "C_TCP_Server_Sender::OnClose( )               presume Client closed connection" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
}

/* ***************************************************************************

Purpose: When Windows schedules and calls this function it indicates that the
CAsyncSocket_Send( ...) method may be called.
The anticipated and primary use is when Send( ...) method solicits error
WSAEWOULDBLOCK. That indicates the current send operation did not
succeed. Some time later, when the TCP/IP system can again
accept send calls, Windows calls this method to indicate ok to send.

Therefore, when this method is called, call the method to send the
buffered data.

*************************************************************************** */

void C_TCP_Server_Sender::OnSend(int nErrorCode)
{
   m_wsa_error = nErrorCode;
   m_on_send_count ++;

   if( m_wsa_error != 0 )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text);
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_Log_Writer::OnSend() __LINE__ %d called wsa error %d %s",
         __LINE__,
         m_wsa_error,
         m_wsa_text);
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
      // I was tempted to put an else here, but decided not.
      // If we get an error and leave sending disabled when we should not have,
      // there there is no method of re-enabling sending.
      // If send should be disable, then sending another buffer will yield 
      // a new would block error and all will be okay.

   if( ! m_buffer_empty )
   {
      Send_Buffered_Data( ); // Send all the buffered data
   }
} // end of: OnSend()

/* ***************************************************************************
  *************************************************************************** */
void C_TCP_Server_Sender::OnAccept( int nErrorCode )
{
   m_on_accept_count ++;
   if( nErrorCode != 0 )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( nErrorCode, m_wsa_text);
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::OnAccept() error in argument %d %s",
         m_wsa_error,
         m_wsa_text);
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
}

/* ***************************************************************************
  *************************************************************************** */
void C_TCP_Server_Sender::OnConnect( int nErrorCode )
{
   m_on_connect_count ++;
   if( nErrorCode != 0 )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( nErrorCode, m_wsa_text);
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::OnConnect() error in argument %d %s",
         m_wsa_error,
         m_wsa_text);
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
}

/* ***************************************************************************
  *************************************************************************** */
void C_TCP_Server_Sender::OnOutOfBandData( int nErrorCode )
{
   m_on_out_of_band_count ++;
   if( nErrorCode != 0 )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( nErrorCode, m_wsa_text);
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::OnOutOfBandData() error in argument %d %s",
         m_wsa_error,
         m_wsa_text);
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
}

/* ***************************************************************************
  *************************************************************************** */
void C_TCP_Server_Sender::OnReceive( int nErrorCode )
{
   m_on_receive_count ++;
   if( nErrorCode != 0 )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( nErrorCode, m_wsa_text);
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::OnReceive() error in argument %d %s",
         m_wsa_error,
         m_wsa_text);
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
}

/* ***************************************************************************

  Purpose: When the TCP/IP send class has reported it would block (cannot send)
  this is called to buffer the data until sending can resume.
  *************************************************************************** */

void C_TCP_Server_Sender::Buffer_A_Message( const void *data_to_send, unsigned __int16 length )

{
   // set ths pointer so the payload packet sequence number can be identified.
   mp_message_buffer = (TD_SEND_BUFFER_DATA *) data_to_send;

#pragma region log_method_entry
   if( m_log_buffered_data )
   {

      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::Buffer_A_Message( ) entered __LINE__ %4d buffering a packet length %d seq# %6d  %8d insert %2d "
         "remove %2d empty %d full %d OnSend( ) count %d m_sending_allowed %d",
         __LINE__,
         length,
         mp_message_buffer->iads_format.header.sequence_number,
         m_buffer_insert_index,
         m_buffer_remove_index,
         m_buffer_empty == true ? "true" : "false",
         m_buffer_full == true ? "true" : "false",
         m_on_send_count,
         m_sending_allowed  );
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
#pragma endregion

   if( !m_buffer_full )
   {
      if( length > 0 && length <= C_IADS_SIZE_TD_MESSAGE_PACKET)  // sanity check
      {
            // save the length of the data
         m_message_buffer[ m_buffer_insert_index].length_to_send = length;
            // load the data into the array
         memcpy( m_message_buffer[ m_buffer_insert_index ].data_to_send.char_format, data_to_send, length);
            // bump up the index and the fill level
         m_buffer_insert_index ++;
         m_buffer_fill_depth ++;
         m_buffer_empty = false;
         if( m_buffer_fill_depth > m_buffer_fill_max_count )
         {
            m_buffer_fill_max_count = m_buffer_fill_depth;
         }

         // When at end of buffer array, loop the index back around to the start
         if( m_buffer_insert_index >= C_TCP_SEND_BUFFER_DEPTH)
         {
            m_buffer_insert_index = 0;
         }

         // When insert == remove, declare buffer full
         if( m_buffer_insert_index == m_buffer_remove_index)
         {
            m_buffer_full = true;
         }
      }
      else // length is bad
      {

#pragma region log_bad_length
         sprintf_s(
            m_text_to_log,
            MAX_LOG_STRING_LENGTH,
            "C_TCP_Server_Sender::Buffer_A_Message send length out of range %d",
            length );
         mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
#pragma endregion

      } // end of: log bad length

   }
   else // buffer is full
   {

      m_lost_message_count ++;

#pragma region log_full
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
      "C_TCP_Server_Sender::Buffer_A_Message( ) buffer full lost payload seq# %8d "
      "length %5d (max %6d) full %d empty %d insert %2d remove %2d count %2d max depth %3d",
      mp_message_buffer->iads_format.header.sequence_number,
      length,
      C_IADS_SIZE_TD_MESSAGE_PACKET,
      m_buffer_full == true ? "true" : "false",
      m_buffer_empty == true ? "true" : "false",
      m_buffer_insert_index,
      m_buffer_remove_index,
      m_buffer_fill_depth,
      C_TCP_SEND_BUFFER_DEPTH );
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
#pragma endregion

   }
} // end of: Buffer_A_Message( )


/* ***************************************************************************
When the TCP/IP send class has reported that sending is now allowed,
this is called to send out the buffered data.
*************************************************************************** */

void C_TCP_Server_Sender::Send_Buffered_Data( )
{
   int  bytes_sent      = 0;
   int  length          = 0;
   int  packets_sent    = 0;
   bool error_detected  = false;
   bool log_it          = false;

#pragma region Log_send
   if( m_log_buffer_send )
   {
      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::Send_Buffered_Data( ) entered __LINE__ %4d OnSend( ) count %d",
         __LINE__,
         m_on_send_count );
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
#pragma endregion

   // This can be called during start up and possibly other times when OnSend( )
   // is called. Skip it all if the buffer is empty.

   if( ! m_buffer_empty)
   {
      do{  // repeat until the buffer is empty or there is an error.
           // A would block indicator is a type of error

            // Get the length, check it, send the data, and get the status.
         length = m_message_buffer[ m_buffer_remove_index].length_to_send;

         if( length > 0 && length <= C_IADS_SIZE_TD_MESSAGE_PACKET ) // sanity check
         {
            bytes_sent = CAsyncSocket::Send( 
               m_message_buffer[ m_buffer_remove_index ].data_to_send.char_format, 
               length, 
               0 );
            m_wsa_error = WSAGetLastError( );

            if( m_wsa_error == 0 )
            {
               packets_sent ++;
               m_buffer_full = false; // one sucessful send == buffer not full.

#pragma region log_good
               if( m_log_buffer_send )
               {
                  sprintf_s(
                     m_text_to_log,
                     MAX_LOG_STRING_LENGTH,
                     "C_TCP_Server_Send_Send_Buffered_Data( )good send _LINE_ %4d seq # %6d WSA error %5d",
                     __LINE__,
                     m_message_buffer[ m_buffer_remove_index].data_to_send.iads_format.header.sequence_number,
                     m_wsa_error );
                  mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
               }
#pragma endregion

            }

#pragma region log error

               // breaking out the compound IF statement made testing easier.
            if( m_wsa_error != WSAEWOULDBLOCK && bytes_sent  != length  ) log_it = true;
            if( m_wsa_error != WSAEWOULDBLOCK && m_wsa_error != 0       ) log_it = true;

            if( m_log_buffer_send || log_it )
            {
               m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text);

               sprintf_s(
                  m_text_to_log,
                  MAX_LOG_STRING_LENGTH,
                  "C_TCP_Server_Sender::Send_Buffered_Data( )_LINE_%4d seq # %6d bytes_sent %5d"
                  " full %d empty %d insert %2d remove %2d count %2d depth %2d WSA error %5d",
                  __LINE__,
                  m_message_buffer[ m_buffer_remove_index].data_to_send.iads_format.header.sequence_number,
                  bytes_sent,
                  m_buffer_full == true ? "true" : "false",
                  m_buffer_empty == true ? "true" : "false",
                  m_buffer_insert_index,
                  m_buffer_remove_index,
                  m_buffer_fill_depth,
                  C_TCP_SEND_BUFFER_DEPTH,
                  m_wsa_error,
                  m_wsa_text   );

              mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
            }
#pragma endregion
         }
         else // length out of range, force error to zero, index is advanced to next 
              // item in buffer effectively deleting the bad data.
         {
            m_wsa_error = 0;
            m_message_buffer[ m_buffer_remove_index ].length_to_send = 0;

#pragma region log bad length
            sprintf_s(
               m_text_to_log,
               MAX_LOG_STRING_LENGTH,
               "C_TCP_Server_Send_Send_Buffered_Data( ) __LINE__ %4d send length was wrong %d",
               __LINE__,
               length );
            mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
#pragma endregion

         }

         // Advance the index only where error == zero. On all other conditions the data was not sent.
         if( m_wsa_error == 0 )
         {
            // set the length to zero to reduce possibility of sending data twice.
            m_message_buffer[ m_buffer_remove_index ].length_to_send = 0;
            m_buffer_remove_index ++;
            m_buffer_fill_depth --;

            // If needed, wrap around to beginning of buffer.
            if( m_buffer_remove_index >= C_TCP_SEND_BUFFER_DEPTH )
            {
               m_buffer_remove_index = 0;
            }

            // If the two index values are equal then buffer is empty.
            if( m_buffer_remove_index == m_buffer_insert_index )
            {
               m_buffer_empty = true;
            }
         } // Log every error but zero and would block
         else if( m_wsa_error!= 0 && m_wsa_error!= WSAEWOULDBLOCK)
         {

#pragma region log_errors

            m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text);
            sprintf_s(
               m_text_to_log,
               MAX_LOG_STRING_LENGTH,
               "C_TCP_Server_Send_Send_Buffered_Data( ) _LINE_ %4d UNEXPECTED WSA ERROR %d %s ",
                __LINE__,
               m_wsa_error,
               m_wsa_text     );
            mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
#pragma endregion

         }
      } while( !m_buffer_empty && m_wsa_error == 0 );

   } // end of: if( ! m_buffer_empty)

   // When there are no errors AND the buffer is empty, resume normal sends
   if( m_wsa_error == 0 && m_buffer_empty)
   {
      m_sending_allowed = true;
   }

#pragma region log exit
   if( m_log_buffer_send)
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text);

      sprintf_s(
         m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::Send_Buffered_Data( ) exit __LINE__ %4d exiting packets sent %3d "
         "buff empty = %s buff full = %s insert %d remove %d wsa = %d %s",

         __LINE__,
         packets_sent,
         m_buffer_empty == true ? "true" : "false",
         m_buffer_full  == true ? "true" : "false",
         m_buffer_insert_index,
         m_buffer_remove_index,
         m_wsa_error,
         m_wsa_text );
         mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }
#pragma endregion

} // end of: Send_Buffered_Data( )



/* ***************************************************************************
   *************************************************************************** */
void C_TCP_Server_Sender::Translate_TCP_STATE( C_TCP_TD_TCP_STATE input, char *output_text )
{

   switch( input)
   {
   case SEND_ENDIAN:
      {
         break;
         strcpy_s( m_tcp_state_text, MAX_TCP_STATE_TEXT, "SEND_ENDIAN");
      }
   case SEND_FRAME:
      {
         strcpy_s( m_tcp_state_text, MAX_TCP_STATE_TEXT, "SEND_FRAME");
         break;
      }
   case SEND_DATA:
      {
         strcpy_s( m_tcp_state_text, MAX_TCP_STATE_TEXT, "SEND_DATA");
         break;
      }

   case SENDER_MUST_EXIT:
      {
         strcpy_s( m_tcp_state_text, MAX_TCP_STATE_TEXT, "SENDER_MUST_EXIT");
         break;
      }
      
   default:
      {
         strcpy_s( m_tcp_state_text, MAX_TCP_STATE_TEXT, "NOT DEFINED");
      }
   }
}

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Sender::Get_All_On_Call_Counts( unsigned int *on_connect,
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
unsigned int C_TCP_Server_Sender::Get_Blocked_Count()
{
   return m_sending_blocked_count;
}

/* ************************************************************************
   ************************************************************************ */
unsigned int C_TCP_Server_Sender::Get_Buffer_Fill_Max_Count()
{
   return m_buffer_fill_max_count;
}

/* ************************************************************************
   ************************************************************************ */
unsigned int C_TCP_Server_Sender::Get_Lost_Message_Count()
{
   return m_lost_message_count;
}

/* ************************************************************************
   ************************************************************************ */
unsigned int C_TCP_Server_Sender::Get_Buffer_Fill_Level()
{
   return m_buffer_fill_depth;
}

/* ************************************************************************
   ************************************************************************ */

int C_TCP_Server_Sender::Class_Receive( char *receive_buffer, unsigned int max_requested, int flags )
{
   int received_count = 0;
   received_count = CAsyncSocket::Receive( receive_buffer, max_requested, flags );
   m_wsa_error = WSAGetLastError();

      // the would block is an expected condition.
   if( m_wsa_error != 0 && m_wsa_error != WSAEWOULDBLOCK )
   {
      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error, m_wsa_text );

      sprintf_s( m_text_to_log,
         MAX_LOG_STRING_LENGTH,
         "C_TCP_Server_Sender::Class_Receive()          requested %d, received %d error code %d %s",
         max_requested,
         received_count,
         m_wsa_error,
         m_wsa_text   );
      mp_C_Log_Writer->Write_Log_File_Entry( m_text_to_log );
   }

   return received_count;
}

/* ************************************************************************
   ************************************************************************ */

void C_TCP_Server_Sender::Reset_Counters()
{
   m_buffer_fill_max_count = 0;
   m_sending_blocked_count = 0;
   m_buffer_fill_max_count = 0;
   m_lost_message_count    = 0;
}


/* ************************************************************************
   ************************************************************************ */
