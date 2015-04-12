// file name: C_TCP_Client.cpp

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
#include "C_TCP_Client.h"

C_TCP_Client::C_TCP_Client(  C_Log_Writer  *logger_pointer,
                             UINT           new_port_number,  
                             const char    *new_ip_address )
{ 

   mp_C_Log_Writer    = logger_pointer;
   m_port_number      = new_port_number;

   size_t new_ip_length = strlen( new_ip_address );
   if( new_ip_length < C_TCP_MAX_IP_ADDRESS_LENGTH && new_ip_length > C_TCP_MIN_IP_ADDRESS_LENGTH )
   {
      errno_t str_status = strcpy_s( m_ip_address, C_TCP_MAX_IP_ADDRESS_LENGTH, new_ip_address );
   }
   else
   {
      sprintf_s( m_log_entry,
         C_TCP_MAX_LOG_WRITE,
         "C_TCP_Client() new_ip_address length of new_ip_length %d exceeds limits %d to %d", 
         new_ip_length,
         C_TCP_MIN_IP_ADDRESS_LENGTH,
         C_TCP_MAX_IP_ADDRESS_LENGTH  );
      mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
      AfxMessageBox( m_log_entry  );

      throw 1;
  
   }

     // These are testing and debugging counters.  They can let the user
     // know if an On*() gets called when it is not expected.  Without these
     // the user would never know.
   m_on_connect_count = 0;
   m_on_receive_count = 0;
   m_on_close_count   = 0;
   m_on_accept_count  = 0;
   m_on_send_count    = 0;

   m_on_out_of_band_count = 0;
   m_payload_packet_count = 0;
   m_emergency_loop_exit  = 0;

   m_sequence_match_count      = 0;
   m_sequence_miss_match_count = 0;
   m_payload_split_count       = 0;

   m_data_valadation         = false;
   m_data_logging_enabled    = false;
   m_log_split_packets       = false;
   m_log_all_packets         = false;
   m_log_sequence_match      = false;
   m_log_sequence_miss_match = false;


     // Presume no split packets on start
   m_carry_over_bytes       = 0;



   mp_search_pointer          = &m_receive_buffer;
   m_expected_sequence_number = 1; // arbitrary

   memset( &m_carry_over_buffer, 0xFF, sizeof( m_carry_over_buffer) );

   
  sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client() constructor done\n             "
      "new port number is %d "
      "new IP address is %s", 
      m_port_number,
      m_ip_address  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );


}  // end of constructor


/*  **********************************************************************
    ********************************************************************** */
C_TCP_Client::~C_TCP_Client()
{
   CAsyncSocket::Close( );
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client()::~C_TCP_Client called"  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}

/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Class_Set_Port_Number( int new_port )
{
   m_port_number = new_port;
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "Class_Set_Port_Number() new port = %d", 
      m_port_number );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}

/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Class_Set_IP_Address( const char *new_ip_address )
{
   int ip_length = strlen( new_ip_address );
   if( ip_length >= C_TCP_MIN_IP_ADDRESS_LENGTH && ip_length < C_TCP_MAX_IP_ADDRESS_LENGTH )
   {
      strcpy_s( m_ip_address, C_TCP_MAX_IP_ADDRESS_LENGTH, new_ip_address );

      sprintf_s( m_log_entry,
         C_TCP_MAX_LOG_WRITE,
         "Class_Set_IP_Address() new ip address is %s",
         m_ip_address );
      mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
   }
   else
   {
      sprintf_s( m_log_entry,
         C_TCP_MAX_LOG_WRITE,
         "Class_Set_IP_Address() new ip address not changed, length out of range = %d",
         ip_length );
      mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
   } 
}

/*   ****************************************************************
     ************************************************************* */

bool C_TCP_Client::Class_Initialize( )
{
   int   wsa_error_code = 0;
   BOOL  winsock_status = FALSE;
   bool  status         = false;

   winsock_status = AfxSocketInit( );
   m_wsa_error_code = WSAGetLastError( );

   winsock_status == 0 ? status = false : status = true;
   if( m_wsa_error_code != 0 ) status = false;

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error_code, m_tcp_error_text );
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Class_Initialize() AfxSocketInit() %s winsock status = %d m_wsa_error_code = %d %s", 
      status == true ? "Success" : "Failed",
      winsock_status,
      m_wsa_error_code,
      m_tcp_error_text  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );

   if( status )
   {
      winsock_status = CAsyncSocket::Create( );
      m_wsa_error_code = WSAGetLastError( );
      ( winsock_status   == 0 || m_wsa_error_code != 0 )  ? status = false : status = true;
   }

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error_code, m_tcp_error_text );
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Class_Initialize Create()          %s winsock_status = %d m_wsa_error_code = %d %s",
      status == true ? "Success" : "Failed",
      winsock_status,
      m_wsa_error_code,
      m_tcp_error_text  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );

   return status;
}

/*   ****************************************************************
     ************************************************************* */

void C_TCP_Client::Class_Close()
{

   CAsyncSocket::ShutDown( );
   CAsyncSocket::Close( );

   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Class_Close() called."  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}

/*   ****************************************************************
  Remember, this is Asynchronous, it does not wait for a sucessful connection.
  Success only means that the connection was sucessfully initiated.
  Method OnConnect() is called when the connection is complete.
     ************************************************************* */

bool C_TCP_Client::Class_Connect( )
{
   bool status = false;

      // Remember BOOL is not really a boolean.  Documentation says not zero is success.
   BOOL winsock_status = CAsyncSocket::Connect( m_ip_address, m_port_number );
   m_wsa_error_code = WSAGetLastError( );

      // think about it.
   if( ( winsock_status > 0 && m_wsa_error_code == 0 ) || m_wsa_error_code == WSAEWOULDBLOCK )
   {
      status = true;
   }

   
   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error_code, m_tcp_error_text );
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Class_Connect()                    port %6d address %s winsock_status = %d m_wsa_error_code = %d %s", 
      m_port_number,
      m_ip_address,
      winsock_status,
      m_wsa_error_code,
      m_tcp_error_text  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);

   return status;
}


/* **************************************************************
Purpose: When this is called, the connection has been established.
For a more robust operation, check the wsa error code.  The app may need to
correct a problem and initiate the connection again. 
For now, just log the results.
There are several errors associated with this method.
I choose to just log them and otherwise ignore them.
****************************************************************** */

void C_TCP_Client::OnConnect( int nErrorCode )
{
   m_wsa_error_code = nErrorCode;
   m_on_connect_count ++;

   m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error_code, m_tcp_error_text );
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::OnConnect()                        m_on_connect_count = %d m_wsa_error_code = %d %s",
      m_on_connect_count,
      m_wsa_error_code,
      m_tcp_error_text  );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
}


/* **************************************************************
Purpose: When this is called, data is waiting to be "received" by this
application. It has already been received by Windows OS and is in the 
Windows buffers.
The only error documented with this method is WSAENETDOWN.
That error is being ignored for simplicity.
****************************************************************** */

void C_TCP_Client::OnReceive( int nErrorCode )
{
   m_on_receive_count ++;
   m_receive_byte_count = CAsyncSocket::Receive( m_receive_buffer.char_array, C_TCP_MAX_RECEIVE_SIZE, 0 );
   m_wsa_error_code = WSAGetLastError( );
   if( m_wsa_error_code != 0)
   {

      m_C_TCP_Format_WSA_Text.Format_WSA_Error_As_Text( m_wsa_error_code, m_tcp_error_text );
      sprintf_s( m_log_entry,
         C_TCP_MAX_LOG_WRITE,
         "C_TCP_Client::OnReceive() m_wsa_error_code %d ",
         m_wsa_error_code,
         m_tcp_error_text  );
      mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
   }
  
   if(  m_data_valadation )
   {
      Validate_Received_Data( );
   }
   else
   {
      // removed the elses
   }
} // end of: OnReceive()


/* **************************************************************
The connection has been closed.
****************************************************************** */

void C_TCP_Client::OnClose( int nErrorCode )
{
   m_wsa_error_code = WSAGetLastError();
   m_on_close_count ++;

   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::OnClose() m_on_close_count = %d m_wsa_error_code = %d",
      m_on_close_count,
      m_wsa_error_code );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
}

/* **************************************************************
 This is only used on the server side. Log all calls.
****************************************************************** */

void C_TCP_Client::OnAccept( int nErrorCode )
{
   m_wsa_error_code = WSAGetLastError( );
   m_on_accept_count ++;
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::OnAccept() m_on_accept_count = %d m_wsa_error_code = %d",
      m_on_accept_count,
      m_wsa_error_code );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
}


/* **************************************************************
 Not used in this implementation.
****************************************************************** */

void C_TCP_Client::OnOutOfBandData( int nErrorCode)
{
   m_wsa_error_code = WSAGetLastError();
   m_on_out_of_band_count ++;
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C lADS Client::OnOutOfBandData() m_on_out_of_band_count = %d m_wsa_error_code = %d ",
      m_on_out_of_band_count,
      m_wsa_error_code );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}


/* **************************************************************
 Indicates sending is now allowed. Not used in this implementation.
****************************************************************** */

void C_TCP_Client::OnSend( int nErrorCode)
{
   m_wsa_error_code = WSAGetLastError( );
   m_on_send_count ++;

   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::OnSend()                           m_on_send_count = %d m_wsa_error_code = %d ",
      m_on_send_count,
      m_wsa_error_code );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}


/* **************************************************************

Purpose: This section logs validates payload packets.
         Caution: those packets are expected to be formated per IADS documents.
         ( Symvionics.com )  But that should be easy to change.
         Look at C_TCP_IADS_Typedefs.h
         It must handle payload packets that have been split across one
         or more TCPlIP packet. It re-assembles the packets, then
         logs the results.
         The reassemble part was a bit difficult.
****************************************************************** */

void C_TCP_Client::Validate_Received_Data( )
{
   unsigned int  true_packet_size        = 0;    // The header word plus four bytes
   bool          get_another             = true;  // keep processing payload packets 
   unsigned int  start_of_payload_packet = 0;     // tracks the beginning of a payload packet.
   unsigned int  end_of_payload_packet   = 0;     // notes the end of a payload packet.

   unsigned int received_sequence_number;

 // m_carry_over_bytes When a payload packet is split across two reads, this has the
 //                    number of bytes that were in the previous read.


   // First Order Of Business, take care of unfinished business!
   //
   // If the carry over count is greater than zero, then the last TCP/IP packet contained
   // one or more bytes of the next payload packet, but not all.
   // carry_over_bytes is the number of bytes of that last payload packet that have
   // been saved in buffer m_carry_over_buffer. At this point, the just read TCP/IP
   // packet contains the remainder of that payload packet.
   //
   // CAREFUL !!
   // I presume the carry over count will be at least four bytes, enough to contain the size of
   // the split packet.

   // WARNING !!
   // I also presume that one payload packet will fit inside a TCP/IP packet.  It will not be
   // split over three or more TCP/IP packets, or more correctly, will not be split over
   // three reads from class CAsyncSocket.

   if( m_carry_over_bytes > 0 )
   {

      true_packet_size = m_carry_over_buffer.iads_structure.header.packet_size + 4;

         // Move sufficient data into m_carry_over_buffer to complete the payload packet
         // make this a memcpy later
      for( ; 
         m_carry_over_bytes < true_packet_size;
         m_carry_over_bytes ++, start_of_payload_packet ++ )
      {
         m_carry_over_buffer.char_array[ m_carry_over_bytes] = m_receive_buffer.char_array[ start_of_payload_packet ];
      }


         // Be certain this code is examining the header of a packet 
         // and not some place else.
      if( m_carry_over_buffer.iads_structure.header.dummy[ 2 ] == DV_HEADER_WORD_SEVEN &&
          m_carry_over_buffer.iads_structure.header.dummy[ 2 ] == DV_HEADER_WORD_EIGHT )
      {

            // check the sequence number to determine if it is correct.
         received_sequence_number = m_carry_over_buffer.iads_structure.header.sequence_number;
         if( m_expected_sequence_number == received_sequence_number  )
         {
            m_expected_sequence_number ++;
            m_sequence_match_count ++;
         }
         else
         {
               // The sequence number was wrong.  Get the expected value for
               // the next payload packet.
            m_sequence_miss_match_count ++;
            m_expected_sequence_number = m_carry_over_buffer.iads_structure.header.sequence_number + 1;
         }
      }
      else
      {
            // The packet header is not valid.    
            // Continue as though there were no carry over data.
         m_carry_over_bytes = 0;
         start_of_payload_packet = 0;
      }
      

   } // end of: if( m_carry_overbytes > 0 )


   do
   {

      /*
         This code PRESUMES that there are at least four bytes of payload packet and
         goes from there.  That means I presume that the OS and its TC/IP code will
         never split a payload packet leaving just 1, 2, or 3 bytes of a partial
         payload packet at the end of the last payload packet.
      */


      mp_search_pointer =  (RECEIVE_TYPE * ) &m_receive_buffer.char_array[ start_of_payload_packet ];

      m_payload_packet_count ++;

         // The first word (four bytes) of the packet specifies the number of payload bytes
         // that follow that first word. Add 4 to get the complete length.
      true_packet_size = mp_search_pointer->iads_structure.header.packet_size + 4;
      end_of_payload_packet = true_packet_size + start_of_payload_packet;

         // If the received buffer contains a complete header, then validate it.
         // Remember, this is a simple validation.
      if( start_of_payload_packet + C_IADS_SIZE_HEADER_PACKET <= ( m_receive_byte_count ) )
      {
         /*
         There are one or more complete payload packets in the TCP/IP packet.
         Ensure that this code is examining a proper payload packet header.
         */

         if( mp_search_pointer->iads_structure.header.dummy[2] == DV_HEADER_WORD_SEVEN &&
            mp_search_pointer->iads_structure.header.dummy[3]  == DV_HEADER_WORD_EIGHT )
         {

            received_sequence_number = mp_search_pointer->iads_structure.header.sequence_number;
            if( m_expected_sequence_number == received_sequence_number )
            {
               m_sequence_match_count ++;
            }
            else
            {
               m_sequence_miss_match_count ++;
            }
            // What ever was discovered, set the expected sequence number
            // to one more than the just received sequence number.
            m_expected_sequence_number = mp_search_pointer->iads_structure.header.sequence_number + 1;

         }
         else
         {
            /*
            Bad packet, skip attempts at recovery for now and start over.
            */
            m_carry_over_bytes = 0;
            return;  // exit on error makes the remaining code simpler
         }
      }


         // The received data contained an exact number of payload packets.
         // All done.
      if( end_of_payload_packet == m_receive_byte_count )
      {
         get_another = false;
      }
            // There is more data after the end of the payload packet, 
            // continue the loop and process the next payload packet.
            // Bump variable start_of_payload_packet up to the next payload packet.
      else if( end_of_payload_packet < m_receive_byte_count )
      {
         get_another = true;
         start_of_payload_packet += true_packet_size;
      }
         // There is insufficient data for another entire packet.
         // Move the partial packet into the carry_over_buffer, exit this
         // method, and wait for the next receive.
      else if( end_of_payload_packet >  m_receive_byte_count  )
      { 
         m_payload_split_count ++;
         get_another = false;

            // m_carry_over_bytes is critical in the next entry
         m_carry_over_bytes = m_receive_byte_count - start_of_payload_packet;
         memcpy( &m_carry_over_buffer, 
                  mp_search_pointer, 
                  m_carry_over_bytes );
      }


   } while( get_another );


} // end of: C_TCP_Client::Validate_Received_Data()

/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Set_Data_Validation_State( bool new_value )
{
   m_data_valadation = new_value;

     // Start from an unknown position
   m_carry_over_bytes       = 0;
   m_payload_packet_count   = 0;

     // This is a rare event, always log it.
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Set_Data_Validation_State()        %s",
      m_data_valadation == true ? "Enable" : "Not Enabled" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
}
/*  **********************************************************************
    ********************************************************************** */

bool C_TCP_Client::Get_Data_Validation_State()
{
   return m_data_valadation;
}
/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Set_Logging_Enabled( bool new_value)
{
   m_data_logging_enabled = new_value;
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client m_data_logging_enabled %d",
      m_data_logging_enabled == true ? "True" : "False" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
}

//  The following methods are used to control data logging.

/*  **********************************************************************
    ********************************************************************** */

bool C_TCP_Client::Get_Logging_Enabled( )
{
   return m_data_logging_enabled;
}

/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Set_Log_Sequence_Match( bool new_value)
{
   m_log_sequence_match = new_value;
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Set_Log_Sequence_Match()   %s",
      m_log_sequence_match == true ? "Log Sequence Match" : "Do Not Log Sequence Match" );

   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
}

/*  **********************************************************************
    ********************************************************************** */
bool C_TCP_Client::Get_Log_Sequence_Match( )
{
   return m_log_sequence_match;
}

/*  **********************************************************************
    ********************************************************************** */
void C_TCP_Client::Set_Log_Miss_Sequence_Match( bool new_value )
{
   m_log_sequence_miss_match = new_value;
   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Set_Log_Miss_Sequence_Match()%s",
      m_log_all_packets == true ? "Log Sequence Miss Match" : "Do Not Log Sequence Miss Match" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}

/*  **********************************************************************
    ********************************************************************** */
bool C_TCP_Client::Get_Log_Miss_Sequence_Match( )
{
   return m_log_sequence_miss_match;
}

/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Set_Log_All_Packets( bool new_value )
{
   m_log_all_packets = new_value;

   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Set_Log_All_Packets()              %s",
      m_log_all_packets == true ? "Log All Packets" : "Not Log All Packets" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
}

/*  **********************************************************************
    ********************************************************************** */
bool C_TCP_Client::Get_Log_All_Packets( )
{
   return m_log_all_packets;
}

/*  **********************************************************************
    ********************************************************************** */
void C_TCP_Client::Set_Log_Split_Packets( bool new_value)
   {
  m_log_split_packets = new_value;

   sprintf_s( m_log_entry,
      C_TCP_MAX_LOG_WRITE,
      "C_TCP_Client::Set_Log_Split_Packets()                   %s",
      m_log_split_packets == true ? "Log Split Packets" : "Do Not Log Split Packets" );
   mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
   }

bool C_TCP_Client::Get_Log_Split_Packets( )
{
   return m_log_split_packets;
}


/*  **********************************************************************
    ********************************************************************** */
void C_TCP_Client::Get_All_On_Call_Counts( unsigned int *on_connect,
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

/*  **********************************************************************
    ********************************************************************** */
void C_TCP_Client::Get_Sequence_Match_Counts( unsigned int *good_match,
                                   unsigned int *bad_match )
{
   *good_match = m_sequence_match_count;
   *bad_match  = m_sequence_miss_match_count;
}

/*  **********************************************************************
    ********************************************************************** */
unsigned int C_TCP_Client::Get_Payload_Split_Count()
{
   return m_payload_split_count;
}

/*  **********************************************************************
    ********************************************************************** */

void C_TCP_Client::Get_IP_Address( char * ip_address, rsize_t max_size )
{
   strcpy_s( ip_address, max_size, m_ip_address );
}


/*  **********************************************************************
    ********************************************************************** */
unsigned int C_TCP_Client::Get_Emergency_Exit_Count()
{
   return m_emergency_loop_exit;
}
/*  **********************************************************************
    ********************************************************************** */

C_TCP_TD_SEND_STATUS C_TCP_Client::Class_Send( const void *data_to_send, int size, int flags)
{
   int chars_sent = 0;
   C_TCP_TD_SEND_STATUS return_status = SEND_OKAY;


   chars_sent = CAsyncSocket::Send( (const void *) data_to_send, size, flags );
   m_wsa_error_code = WSAGetLastError( );

   if( chars_sent == size && m_wsa_error_code == 0 )
   {
      return SEND_OKAY;
   }
   else if( m_wsa_error_code == WSAEWOULDBLOCK )
   {
      return SEND_BLOCKED;
   }
   else
   {
      sprintf_s( m_log_entry,
         C_TCP_MAX_LOG_WRITE,
         "C_TCP_Client::Class_Send()                       unexpected error: %d",
         m_wsa_error_code );
      mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
      return OTHER_ERROR;
   }
}

/*  **********************************************************************
    ********************************************************************** */



//
//#pragma region log_miss
//         if( m_log_sequence_miss_match )
//         {
//            sequence_difference =  mp_search_pointer->iads_structure.header.sequence_number - m_expected_sequence_number;
//            sprintf_s( m_log_entry,
//               C_TCP_MAX_LOG_WRITE,
//               "Validate_Received_Data()__LINE__ %d bad sequence, expected %8d found %8d difference %d",
//               __LINE__,
//               m_expected_sequence_number,
//               mp_search_pointer->iads_structure.header.sequence_number,
//               sequence_difference );
//            mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
//
//         m_curr_header_packet_size     = mp_search_pointer->iads_structure.header.packet_size;
//         m_curr_header_sequence_number = mp_search_pointer->iads_structure.header.sequence_number;
//         m_curr_header_packets_sent    = mp_search_pointer->iads_structure.header.packets_sent;
//         m_curr_header_packets_lost    = mp_search_pointer->iads_structure.header.loss_or_overflow;
//         m_curr_header_dummy_0_5555    = mp_search_pointer->iads_structure.header.dummy[0];
//         m_curr_header_dummy_1_6666    = mp_search_pointer->iads_structure.header.dummy[1];
//         m_curr_header_dummy_2_7777    = mp_search_pointer->iads_structure.header.dummy[2];
//         m_curr_header_dummy_3_ffff    = mp_search_pointer->iads_structure.header.dummy[3];
//
//          sprintf_s( m_log_entry,
//               C_TCP_MAX_LOG_WRITE,
//               "Previous header data: %8X %8X %8X %8X %8X %8X %8X %8X ",
//                m_prev_header_packet_size,
//                m_prev_header_sequence_number,
//                m_prev_header_packets_sent,
//                m_prev_header_packets_lost,
//                m_prev_header_dummy_0_5555,
//                m_prev_header_dummy_1_6666,
//                m_prev_header_dummy_2_7777,
//                m_prev_header_dummy_3_ffff  );
//            mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
//
//          sprintf_s( m_log_entry,
//               C_TCP_MAX_LOG_WRITE,
//               "current  header data: %8X %8X %8X %8X %8X %8X %8X %8X ",
//                m_curr_header_packet_size,
//                m_curr_header_sequence_number,
//                m_curr_header_packets_sent,
//                m_curr_header_packets_lost,
//                m_curr_header_dummy_0_5555,
//                m_curr_header_dummy_1_6666,
//                m_curr_header_dummy_2_7777,
//                m_curr_header_dummy_3_ffff  );
//            mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
//
//         m_prev_header_packet_size     = m_curr_header_packet_size;
//         m_prev_header_sequence_number = m_curr_header_sequence_number;
//         m_prev_header_packets_sent    = m_curr_header_packets_sent;
//         m_prev_header_packets_lost    = m_curr_header_packets_lost;
//         m_prev_header_dummy_0_5555    = m_curr_header_dummy_0_5555;
//         m_prev_header_dummy_1_6666    = m_curr_header_dummy_1_6666;
//         m_prev_header_dummy_2_7777    = m_curr_header_dummy_2_7777;
//         m_prev_header_dummy_3_ffff    = m_curr_header_dummy_3_ffff;
//
//
//         }
//#pragma endregion


//
//#pragma region log_too_big
//         if( m_data_logging_enabled )
//         {
//            sprintf_s( m_log_entry,
//               C_TCP_MAX_LOG_WRITE,
//               "C_TCP_Client::Validate_Received_Data                   __LINE__ %d payload size too large %d limit is %d",
//               __LINE__,
//               true_packet_size,
//               C_IADS_LARGEST_PAYLOAD_SIZE  );
//            mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry);
//         }
//
//
//
//#pragma endregion

//#pragma region log_matches
//         if( m_log_sequence_match)
//         {
//            sprintf_s( m_log_entry,
//               C_TCP_MAX_LOG_WRITE,
//               "Validate_Received_Data()__LINE__ %d              good sequence, expected %8d found %8d ",
//               __LINE__,
//               m_expected_sequence_number,
//               mp_search_pointer->iads_structure.header.sequence_number );
//            mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
//         }
//#pragma endregion


//#pragma region log_all
//        // Before continuing, if desired, log some info about the just 
//        // discovered payload packet.  OR, if the sequence number does
//        // not match and it is being logged, then log some data.
//      if( m_log_all_packets ||
//          m_log_sequence_miss_match && sequence_difference != 0 )
//      {
//         sprintf_s( m_log_entry,
//            C_TCP_MAX_LOG_WRITE,
//            "Validate_Received_Data() __LINE__ %d               %s recvd count %6d offset %5d next_offset %5d size %5d seq # %7d dummy[3] %08x first tag %3d ",
//            __LINE__,
//            sequence_difference != 0 ? "Miss_match" : "Match OK  ",
//            m_receive_byte_count,
//            buffer_offset,
//            next_buffer_offset,
//            mp_search_pointer->iads_structure.header.packet_size,
//            mp_search_pointer->iads_structure.header.sequence_number,
//            mp_search_pointer->iads_structure. header.dummy[ 3 ],
//            mp_search_pointer->iads_structure. body->tag );
//         mp_C_Log_Writer->Write_Log_File_Entry( m_log_entry );
//      }
//#pragma endregion