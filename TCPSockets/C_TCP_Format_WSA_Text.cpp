//  file name:  C_TCP_Format_WSA_Text.cpp


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
#include <string.h>
#include <WinSock2.h>
#include "C_TCP_Format_WSA_Text.h"

C_TCP_Format_WSA_Text::C_TCP_Format_WSA_Text( )
{
}
C_TCP_Format_WSA_Text::~C_TCP_Format_WSA_Text()
{
}

void C_TCP_Format_WSA_Text::Format_WSA_Error_As_Text( int wsa_error, char*  wsa_text )
{

   switch(wsa_error)
   {
   case 0:
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "OK" );
      break;
    case WSAEFAULT:       //  10014
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The IpSockAddrLen argument is incorrect.");
      break;
  case WSAEINVAL:         //  10022
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The socket is already bound to an address.");
      break;
  case WSAEINPROGRESS:
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "A blocking operation is already in progress.");
      break;
  case WSAEMFILE:         //  10024
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "No more file descriptors are available.");
      break;
  case WSAEWOULDBLOCK:    //  10035
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "Data not sent, wait for OnSend( ) and try again.");
      break;
   case WSAENOTSOCK:      //  10038
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The descriptor is a file, not a socket.");
      break;
   case WSAEDESTADDRREQ:  //  10039
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "A destination address is required.");
      break;
   case WSAEAFNOSUPPORT:  //  10047
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "Addresses in the specified family cannot be used with this socket.");
      break;
   case WSAEADDRINUSE:   //  10048
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The specified address is already in use.");
      break;
   case WSAEADDRNOTAVAIL: // 10049
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The specified address is not available from the local machine");
      break;
   case WSAENETUNREACH:   // 10051
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The network cannot be reached from this host at this time.");
      break;
   case WSAECONNABORTED: //  10053
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "Software abort.");
      break;
  case WSAENOBUFS:       //  10055
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "No buffer space is available. The socket cannot be connected.");
      break;
   case WSAEISCONN:      //  10056
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The socket is already connected.");
      break;
   case WSAENOTCONN:     //  10057
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The socket is not connected.");
      break;
   case WSAETIMEDOUT:   //  10060
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The attempt to connect timed out without establishing a connection.");
      break;
   case WSAECONNREFUSED:  // 10061
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The attempt to connect was forcefully rejected.");
      break;
   case WSAENETDOWN:
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "The network is down.");
      break;
   case WSANOTINITIALISED:   //  10093
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "Not Initialized.  WSAStartup not performed.");
      break;
   default:
      strcpy_s( wsa_text, MAX_WSA_ERROR_TEXT, "No translation for this error");
      break;
   }




}