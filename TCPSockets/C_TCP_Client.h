// file name: file name C_TCP_Client.h

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


/* ******************************************************************
This is the dot H file for the client program. It is an application
created primarily to test the Server code.  It connects to the server's
socket and consumes the data output.  The owner can set a flag causing
it to validate the payload packet with method: Validate_Received_Data();
The owner can set other flags to log various performance items.

********************************************************************* */

#pragma once
#include "afxsock.h"
#include "C_TCP_Constants.h"
#include "C_Log_Writer.h"
#include "C_TCP_IADS_Typedefs.h"
#include "C_TCP_Format_WSA_Text.h"



class C_TCP_Client: public CAsyncSocket
{
public:
     // First argugment is pointer to the logging object.  
     // The others should be obvious.
     // Constructor can throw exception 1 (number 1). Check log file.
     // Additional throws should be added before using code for a product.
     // Indeed, check log no matter what.
   C_TCP_Client( C_Log_Writer *logger_pointer   = NULL,
                 UINT          new_port_number  = C_TCP_DEFAULT_SOCKET_PORT,
                 const char   *new_ip_address   = &C_TCP_DEFAULT_IP_ADDRESS[0] );

  ~C_TCP_Client();

      // These are the virtual methods of base class CAsyncSocket that must
      // be declared here.  Not all are used.
   virtual void OnConnect(       int nErrorCode );
   virtual void OnReceive(       int nErrorCode );
   virtual void OnClose(         int nErrorCode );
   virtual void OnAccept(        int nErrorCode );
   virtual void OnOutOfBandData( int nErrorCode );
   virtual void OnSend(          int nErrorCode );

   void Class_Set_Port_Number( int new_port);
   void Class_Set_IP_Address( const char *new_ip_address);
   bool Class_Initialize( );
   bool Class_Connect( );
   void Class_Close( );
   // Send the data to the server. Note that flags is optional
   C_TCP_TD_SEND_STATUS Class_Send( const void *data_to_send, int size, int flags = 0);
   


      // Methods used to enable and disable various logging 
      // Provides for caller to determine the current logging status.
   void Set_Logging_Enabled( bool new_value );
   bool Get_Logging_Enabled( );

   void Set_Data_Validation_State( bool new_value );
   bool Get_Data_Validation_State();

   void Set_Log_Sequence_Match( bool new_value );
   bool Get_Log_Sequence_Match( );

   void Set_Log_Miss_Sequence_Match( bool new_value );
   bool Get_Log_Miss_Sequence_Match( );

   void Set_Log_All_Packets( bool new_value);
   bool Get_Log_All_Packets( );

      // Split means a payload packet was split across two TCP/IP packets.
   void Set_Log_Split_Packets( bool new_value);
   bool Get_Log_Split_Packets( );

   void Get_IP_Address( char * ip_address, rsize_t max_size );

      // Number of payload packets split.
   unsigned int Get_Payload_Split_Count();
   unsigned int Get_Emergency_Exit_Count();

   void Get_Sequence_Match_Counts( unsigned int *sequence_match,
                                   unsigned int *sequence_miss_match );                                 

   void Get_All_On_Call_Counts( unsigned int *on_connect,
                                unsigned int *on_receive,
                                unsigned int *on_close,
                                unsigned int *on_accept,
                                unsigned int *on_out_of_band,
                                unsigned int *on_send );
   // private methods
private:

       // Method to examine data and log results.  When disabled
       // the received data is not examined.  This client application
       // consumes the data allowing the server to run. 
   void Validate_Received_Data( );

   // public variables
public: 

   // private variables
private:

     // The logger is passed in from the owner.
   C_Log_Writer *mp_C_Log_Writer;

      // Accepts the WSA error value and provides some text for the log.
   C_TCP_Format_WSA_Text m_C_TCP_Format_WSA_Text;

   unsigned int  m_port_number;
   C_TCP_TD_IP_ADDRESS m_ip_address;

      // When the WSA error is translated to text, that text is put here.
   char m_tcp_error_text[ MAX_WSA_ERROR_TEXT ];

   BOOL m_winsock_status;  // some Windows methods require this BOOL return type.

   int  m_wsa_error_code;

       // Build log entries here
   char m_log_entry[ C_TCP_MAX_LOG_WRITE ];

      // The number of times each On*() method is called
   unsigned int  m_on_connect_count;
   unsigned int  m_on_receive_count;
   unsigned int  m_on_close_count;
   unsigned int  m_on_accept_count;
   unsigned int  m_on_out_of_band_count;
   unsigned int  m_on_send_count;

      // used to re-assemble payload packets split across TCP packets.
   unsigned int  m_receive_byte_count;
   unsigned int  m_carry_over_bytes;
   unsigned int  m_payload_packet_count;

       // Count of characters saved when a payload packet is split across two TCP packets.
   unsigned int m_saved_bytes;


   // Receive the data as char[], examine it as a structure.
   // Don't forget that m_receive_buffer is a structure, not a pointer.
   union RECEIVE_TYPE 
   {
      char char_array [ C_TCP_MAX_RECEIVE_SIZE ];
      C_IADS_TD_FIXED_PARAMETER_PACKET iads_structure;
   } m_receive_buffer;

      // Used to search the received data for the beginning of a payload packet. 
      // One TCP packet may contain multiple payload packets AND
      // payload packets can be split across two TCP packets.  When synchronization
      // is lost, a search is required.
   RECEIVE_TYPE *mp_search_pointer;

      // When a payload packed is split across to TCP/IP packets, the first part 
      // of a partial payload packet is placed here so the next receive will
      // not obliterate it.  After the split payload packet is assembled and 
      // processed, the code returns to the main buffer to process remaining
      // payload packets.
   RECEIVE_TYPE m_carry_over_buffer;

      // Track the expected sequence number, should increment by one each payload packet.
   unsigned int m_expected_sequence_number;

      // some statistics to display
   unsigned int m_sequence_match_count;      // the sequence counter was correct
   unsigned int m_sequence_miss_match_count; // the sequence counter was not correct
   unsigned int m_payload_split_count;       // How often a payload packet was split across
                                             // TCP packets.
   unsigned int m_emergency_loop_exit;       // How often was the emergency exit taken.


      // controls logging / debug data sent to the log file
   bool m_data_logging_enabled;
   bool m_log_sequence_match;
   bool m_log_sequence_miss_match;
   bool m_log_all_packets;
   bool m_log_split_packets;
   bool m_data_valadation;         // true  = validate the received payload packets, 
                                   // false = ignore them

   unsigned int m_prev_header_packet_size;
   unsigned int m_prev_header_sequence_number;
   unsigned int m_prev_header_packets_sent;
   unsigned int m_prev_header_packets_lost;
   unsigned int m_prev_header_dummy_0_5555;
   unsigned int m_prev_header_dummy_1_6666;
   unsigned int m_prev_header_dummy_2_7777;
   unsigned int m_prev_header_dummy_3_ffff;

   unsigned int m_curr_header_packet_size;
   unsigned int m_curr_header_sequence_number;
   unsigned int m_curr_header_packets_sent;
   unsigned int m_curr_header_packets_lost;
   unsigned int m_curr_header_dummy_0_5555;
   unsigned int m_curr_header_dummy_1_6666;
   unsigned int m_curr_header_dummy_2_7777;
   unsigned int m_curr_header_dummy_3_ffff;

};