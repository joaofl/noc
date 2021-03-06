/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "noc-address.h"
#include "ns3/address.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/noc-address.h"
#include <iomanip>
#include <iostream>
#include <cstring>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NOCAddress");

ATTRIBUTE_HELPER_CPP (NOCAddress);

#define ASCII_a (0x41)
#define ASCII_z (0x5a)
#define ASCII_A (0x61)
#define ASCII_Z (0x7a)
#define ASCII_COLON (0x3a)
#define ASCII_ZERO (0x30)

/**
 * Converts a char to lower case.
 * \param c the char
 * \returns the lower case
 */
//static char
//AsciiToLowCase (char c)
//{
//  NS_LOG_FUNCTION (c);
//  if (c >= ASCII_a && c <= ASCII_z) {
//      return c;
//    } else if (c >= ASCII_A && c <= ASCII_Z) {
//      return c + (ASCII_a - ASCII_A);
//    } else {
//      return c;
//    }
//}


NOCAddress::NOCAddress ()
{
  NS_LOG_FUNCTION (this);
  m_address[0] = 0;
  m_address[1] = 0;
  m_address[2] = 0;
  m_address[3] = 0;
}

void 
NOCAddress::CopyFrom (const uint8_t buffer[4])
{
  NS_LOG_FUNCTION (this << &buffer);
  std::memcpy (m_address, buffer, 4);
}

void 
NOCAddress::CopyTo (uint8_t buffer[4]) const
{
  NS_LOG_FUNCTION (this << &buffer);
  std::memcpy (buffer, m_address, 4);
}
//
//bool 
//NOCAddress::IsMatchingType (const Address &address)
//{
//  NS_LOG_FUNCTION (&address);  
//  return address.CheckCompatible (GetType (), 6);
//}
NOCAddress::operator Address () const
{
  return ConvertTo ();
}
Address 
NOCAddress::ConvertTo (void) const
{
  NS_LOG_FUNCTION (this);
  return Address (GetType (), m_address, 4);
}
NOCAddress 
NOCAddress::ConvertFrom (const Address &address)
{
  NS_LOG_FUNCTION (&address);
  NS_ASSERT (address.CheckCompatible (GetType (), 4)); //64 bits, 4 bytes
  NOCAddress retval;
  address.CopyTo (retval.m_address);
  return retval;
}

//NOCAddress 
//NOCAddress::Allocate (void)
////NOCAddress::Allocate (uint32_t, int32_t, int32_t, uint8_t, uint8_t)
//{
//  NS_LOG_FUNCTION_NOARGS ();
//  static uint64_t id = 0;
//  id++;
//  NOCAddress address;
//  address.m_address[0] = (id >> 24) & 0xff;
//  address.m_address[1] = (id >> 16) & 0xff;
//  address.m_address[2] = (id >> 8) & 0xff;
//  address.m_address[3] = (id >> 0) & 0xff;
//  
//  return address;
//}

uint8_t 
NOCAddress::GetType (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static uint8_t type = Address::Register ();
  return type;
}

std::ostream& operator<< (std::ostream& os, const NOCAddress & address)
{
//  uint8_t ad[6];
//  address.CopyTo (ad);
//
//  os.setf (std::ios::hex, std::ios::basefield);
//  os.fill ('0');
//  for (uint8_t i=0; i < 5; i++) 
//    {
//      os << std::setw (2) << (uint32_t)ad[i] << ":";
//    }
//  // Final byte not suffixed by ":"
//  os << std::setw (2) << (uint32_t)ad[5];
//  os.setf (std::ios::dec, std::ios::basefield);
//  os.fill (' ');
  return os;
}

std::istream& operator>> (std::istream& is, NOCAddress & address)
{
//  std::string v;
//  is >> v;
//
//  std::string::size_type col = 0;
//  for (uint8_t i = 0; i < 6; ++i)
//    {
//      std::string tmp;
//      std::string::size_type next;
//      next = v.find (":", col);
//      if (next == std::string::npos)
//        {
//          tmp = v.substr (col, v.size ()-col);
//          address.m_address[i] = strtoul (tmp.c_str(), 0, 16);
//          break;
//        }
//      else
//        {
//          tmp = v.substr (col, next-col);
//          address.m_address[i] = strtoul (tmp.c_str(), 0, 16);
//          col = next + 1;
//        }
//    }
  return is;
}


} // namespace ns3
