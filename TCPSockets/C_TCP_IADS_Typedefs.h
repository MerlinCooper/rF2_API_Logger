
// File name lADS_TCP_Typedefs.h

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
This header file was originaly created to work in conjunction with
a display system and its unique payload packet structure.
No software from that application was used, just the basic structure.  
It should be relatively easy to change the structure for your
particular needs.
******************************************************************* */

   // If the size parameter in the payload packet says the payload is larger 
   // than this, then something that looks like a payload but is not has
   // been found.
const unsigned int C_IADS_LARGEST_PAYLOAD_SIZE = 3000;

  // The payload packet structure defined by Symvionics.
typedef struct
{
   unsigned _int32 packet_size;       // Number of bytes that follow this item.  
                                      // Increment by6 4 to get actual byte count.
   unsigned _int32 sequence_number;   // incremented by 1 each payload packet sent
   unsigned _int32 packets_sent;      // Total Packets sent, ignored, but I increment it anywah
   unsigned _int32 loss_or_overflow;  // Total Packets dropped, not used
   unsigned _int32 dummy [4] ;        // Symvionics reserves for future use, I load the data
                                      // with values to ensure the header has been found and
                                      // not some similiar data.
}  C_IADS_TD_PACKET_HEADER;

const int C_IADS_SIZE_HEADER_PACKET = sizeof( C_IADS_TD_PACKET_HEADER);

const int C_IADS_SIZE_OF_TCP_HEADER_SIZE = sizeof( unsigned _int32 );

   // The dummy fields in the header are not used by lADS.
   // For debugging I will use them per these constants, index values into the 
   // array that comprises the dummy section.
   // Read the names as Debug_Zero through Debug_Three
const int DO_All_FIVES  = 0; // The messge number, a double check - - -
const int D1_ALL_SIXES  = 1; // The number of parameters output
const int D2_ALL_SEVENS = 2; // The number of times this message has been discovered
const int D3_ALL_FFFF   = 3;

   // Read the names as Debug_Value_All_Fives, etc.
const int DV_HEADER_WORD_FIVE    = 0x55555555;
const int DV_HEADER_WORD_SIX     = 0x66666666;
const int DV_HEADER_WORD_SEVEN   = 0x77777777;
const int DV_HEADER_WORD_EIGHT   = 0xFFFFFFFF;

   // Each parameter resides in a group of three 32 bit integers
   // Tag number identifies the parameter
   // size says how many bytes it occupies
   // followed by value.  In this example, the value is always 32 bits
   //
   // Those 32 value bits can be int (signed or unsigned) or float.
   // The union provies proper access.
typedef struct
{
   unsigned _int32 tag;
   unsigned _int32 size;
   union { float f; _int32 i; unsigned _int32 ui;} value;
} C_IADS_TD_IADS_PARAMETER_BODY;

  // The size of each parameter.
const int C_IADS_SIZE_IADS_PARAMETER_BODY = sizeof( C_IADS_TD_IADS_PARAMETER_BODY );
                                            
/* *********************************************
The real application has many differing sizes.  This test/demo application
only needs one.
********************************************* */
typedef struct
{
   C_IADS_TD_PACKET_HEADER header;
   C_IADS_TD_IADS_PARAMETER_BODY body[ C_TCP_MESSAGE_GREATEST_PARAMETER_COUNT ];
} C_IADS_TD_FIXED_PARAMETER_PACKET;

const int C_IADS_SIZE_TD_MESSAGE_PACKET = sizeof( C_IADS_TD_FIXED_PARAMETER_PACKET );

typedef struct
{
   union
   {
      C_IADS_TD_FIXED_PARAMETER_PACKET iads_format;
      char                      char_format[ C_IADS_SIZE_TD_MESSAGE_PACKET ];
   } both;
}  C_IADS_TD_TEST_PAYLOAD_PACKET;

const unsigned int C_IADS_SIZE_OF_PAYLOAD_PACKET = sizeof( C_IADS_TD_TEST_PAYLOAD_PACKET );

   // Precalculate the size of a packet that has only one parameter.
const unsigned int C_IADS_ONE_PARAMETER_PACKET_SIZE = C_IADS_SIZE_HEADER_PACKET + C_IADS_SIZE_IADS_PARAMETER_BODY;