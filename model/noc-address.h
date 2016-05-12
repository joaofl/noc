/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 João Loureiro
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
 * Author: João Loureiro <joflo@isep.ipp.pt>
 */
#ifndef NOC_ADDRESS_H
#define NOC_ADDRESS_H

#include <stdint.h>
#include <ostream>
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

class Address;

/**
 * \ingroup address
 * 
 * \brief an usefull addressing scheme for NoC
 *
 * \see attribute_NOCAddress
 */
class NOCAddress
{
public:
  NOCAddress ();
  /**
   * \param str a string representing the new NOCAddress
   *
   * The format of the string is "nnnnn:xxxx:yyyy:w:d"
   * where: 
   *    n is a serial number that identifies each net-device uniquely
   *    x is the x coordinate of the node in which that netdevice is installed
   *    y is the y coordinate of the node in which that netdevice is installed
   *    w is the network to which that specific netdevice belongs to
   *    d is the direction to which that netdevice is pointing to (N=1 S=2 E=3 W=4)
   */
  NOCAddress (const char *str);

  /**
   * \param buffer address in network order
   *
   * Copy the input address to our internal buffer.
   */
  void CopyFrom (const uint8_t buffer[4]);
  /**
   * \param buffer address in network order
   *
   * Copy the internal address to the input buffer.
   */
  void CopyTo (uint8_t buffer[4]) const;

  /**
   * \returns a new Address instance
   *
   * Convert an instance of this class to a polymorphic Address instance.
   */
  operator Address () const;
  /**
   * \param address a polymorphic address
   * \returns a new NOCAddress from the polymorphic address
   * 
   * This function performs a type check and asserts if the
   * type of the input address is not compatible with an
   * NOCAddress.
   */
  static NOCAddress ConvertFrom (const Address &address);
  /**
   * \param address address to test
   * \returns true if the address matches, false otherwise.
   */
  static bool IsMatchingType (const Address &address);
  /**
   * Allocate a new NOCAddress.
   * 
   * \returns newly allocated NOCAddress 
   */
  static NOCAddress Allocate (void);
//  static NOCAddress Allocate (uint32_t, int32_t, int32_t, uint8_t, uint8_t);


  typedef void (* TracedCallback)(const NOCAddress value);
  
private:
    
   uint8_t m_address[4];
    
  /**
   * \returns a new Address instance
   *
   * Convert an instance of this class to a polymorphic Address instance.
   */
  Address ConvertTo (void) const;

  /**
   * \brief Return the Type of address.
   * \return type of address
   */
  static uint8_t GetType (void);

  /**
   * \brief Equal to operator.
   *
   * \param a the first operand
   * \param b the first operand
   * \returns true if the operands are equal
   */
  friend bool operator == (const NOCAddress &a, const NOCAddress &b);

  /**
   * \brief Not equal to operator.
   *
   * \param a the first operand
   * \param b the first operand
   * \returns true if the operands are not equal
   */
  friend bool operator != (const NOCAddress &a, const NOCAddress &b);

  /**
   * \brief Less than operator.
   *
   * \param a the first operand
   * \param b the first operand
   * \returns true if the operand a is less than operand b
   */
  friend bool operator < (const NOCAddress &a, const NOCAddress &b);

  /**
   * \brief Stream insertion operator.
   *
   * \param os the stream
   * \param address the address
   * \returns a reference to the stream
   */
  friend std::ostream& operator<< (std::ostream& os, const NOCAddress & address);

  /**
   * \brief Stream extraction operator.
   *
   * \param is the stream
   * \param address the address
   * \returns a reference to the stream
   */
  friend std::istream& operator>> (std::istream& is, NOCAddress & address);

//  uint8_t m_address[6]; //!< address value
  


    //total size 152 bits, 19 bytes
    
//typedef uint16_t NOCNetDeviceAddress;
  
};

ATTRIBUTE_HELPER_HEADER (NOCAddress);

inline bool operator == (const NOCAddress &a, const NOCAddress &b)
{
    if  ((a.m_address[0] == b.m_address[0]) &&
        (a.m_address[1] == b.m_address[1])  &&
        (a.m_address[2] == b.m_address[2])  &&
        (a.m_address[3] == b.m_address[3]) )
        return 1;
    else return 0;  
            
//  return memcmp (a.m_address, b.m_address, 6) == 0;
}
inline bool operator != (const NOCAddress &a, const NOCAddress &b)
{
    if  ((a.m_address[0] != b.m_address[0]) ||
        (a.m_address[1] != b.m_address[1])  ||
        (a.m_address[2] != b.m_address[2])  ||
        (a.m_address[3] != b.m_address[3]) )
        return 1;
    else return 0;    
}
inline bool operator < (const NOCAddress &a, const NOCAddress &b)
{
  return memcmp (a.m_address, b.m_address, 4) < 0;
}

std::ostream& operator<< (std::ostream& os, const NOCAddress & address);
std::istream& operator>> (std::istream& is, NOCAddress & address);

} // namespace ns3

#endif /* NOC_ADDRESS_H */
