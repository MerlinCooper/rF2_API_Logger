// file name C_TCP_Constants.h

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

#pragma once

const UINT C_TCP_DEFAULT_SOCKET_PORT = 49000;
  // The real application can be started multiple times.  Each is dedicated
  // to a "string" that ranges from 1 to 12.  The default is string 01.
const UINT C_TCP_TEST_STRING_OFFSET  = 1;

  // Length of character array that will hold the IP address
const unsigned int C_TCP_MIN_IP_ADDRESS_LENGTH =  7;  // 1.1.1.1 plus null is 7 characters.
const unsigned int C_TCP_MAX_IP_ADDRESS_LENGTH = 64;

typedef char C_TCP_TD_IP_ADDRESS[ C_TCP_MAX_IP_ADDRESS_LENGTH ];

  // Default IP address
const char C_TCP_DEFAULT_IP_ADDRESS[] = ("127.0.0.1");
//const char C_TCP_DEFAULT_IP_ADDRESS[] = ("192.168.2.4");

  // Length of char array used when translating a WSA error into text.
const unsigned int C_TCP_WSA_ERROR_MAX_TEXT_LENGTH = 32;

   // The send never reads anything from lADS, HOWEVER:
   // In order to get the close status when lADS closes the socket
   // it must have a read posted. Create the read buffer this size
   // and use it to post the read.
const int C_TCP_SENDER_READ_BUFFER = 16;

   // Set some artibrary values that are needed in several places.
const int C_TCP_MAX_RECEIVE_SIZE = 32000;
const int C_TCP_MAX_LOG_WRITE = 256;

const int C_TCP_MESSAGE_GREATEST_PARAMETER_COUNT = 100;

  // States of the FSM in the server manager code
typedef    enum  C_TCP_TD_MANAGER_STATE
   {
      NOT_INITIALIZED,
      INITIALIZED_NOT_LISTENING,
      LISTENING_FOR_CLIENT,
      CLOSING,
   };

const size_t C_TCP_MAX_ENUM_TEXT_LENGTH = 30;

   // In the event that data sends exceed the Windows capacity to send
   // TCP/IP packets, the data must be buffered. This is the depth
   // of that buffering.
const int C_TCP_SEND_BUFFER_DEPTH = 80;

const int C_TCP_LITTLE_ENDIAN  = 1;
const int C_TCP_BIG_ENDIAN     = 2;

   // The two types of payload packets
const int C_IADS_TAG_VALUE_PAIR = 0;  // two tag numbers followed by two values
const int C_IADS_TAG_SIZE_VALUE = 1;  // one tag, one size, and one value


// The state of the TCP sender function.
enum C_TCP_TD_TCP_STATE
{
   SEND_ENDIAN,
   SEND_FRAME,
   SEND_DATA,
   SENDER_MUST_EXIT
};

enum C_TCP_TD_SEND_STATUS
{
   READY_TO_SEND,
   SEND_OKAY,
   SEND_FAIL,
   SEND_BLOCKED,
   SENDER_READY_TO_EXIT,
   SENDER_HAS_EXITED,
   SENDER_NOT_READY,
   OTHER_ERROR
};
