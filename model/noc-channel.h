/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 * 
 * 
 * Adapted by: João Loureiro <joflo@isep.ipp.pt>
 */



#ifndef NOC_CHANNEL_H
#define NOC_CHANNEL_H

#include <list>
#include "ns3/channel.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class NOCNetDevice;
class Packet;

/**
 * \ingroup point-to-point
 * \brief Simple Point To Point Channel.
 *
 * This class represents a very simple point to point channel.  Think full
 * duplex RS-232 or RS-422 with null modem and no handshaking.  There is no
 * multi-drop capability on this channel -- there can be a maximum of two 
 * point-to-point net devices connected.
 *
 * There are two "wires" in the channel.  The first device connected gets the
 * [0] wire to transmit on.  The second device gets the [1] wire.  There is a
 * state (IDLE, TRANSMITTING) associated with each wire.
 */
class NOCChannel : public Channel 
{
public:
  static TypeId GetTypeId (void);

  /**
   * \brief Create a NOCChannel
   *
   * By default, you get a channel that
   * has zero transmission delay.
   */
  NOCChannel ();

  /**
   * \brief Attach a given netdevice to this channel
   * \param device pointer to the netdevice to attach to the channel
   */
  void Attach (Ptr<NOCNetDevice> device);

  /**
   * \brief Transmit a packet over this channel
   * \param p Packet to transmit
   * \param src Source NOCNetDevice
   * \param txTime Transmit time to apply
   * \returns true if successful (currently always true)
   */
  virtual bool TransmitStart (Ptr<Packet> p, Ptr<NOCNetDevice> src, Time txTime);
  
  virtual bool TransmitSignalStart (Ptr<Packet> p, Ptr<NOCNetDevice> src, Time txTime);
  
  /**
   * \brief Get number of devices on this channel
   * \returns number of devices on this channel
   */
  virtual uint32_t GetNDevices (void) const;

  /*
   * \brief Get NOCNetDevice corresponding to index i on this channel
   * \param i Index number of the device requested
   * \returns Ptr to NOCNetDevice requested
   */
  Ptr<NOCNetDevice> GetNOCDevice (uint32_t i) const;

  /*
   * \brief Get NetDevice corresponding to index i on this channel
   * \param i Index number of the device requested
   * \returns Ptr to NetDevice requested
   */
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

protected:
  /*
   * \brief Get the delay associated with this channel
   * \returns Time delay
   */
  Time GetDelay (void) const;

  /*
   * \brief Check to make sure the link is initialized
   * \returns true if initialized, asserts otherwise
   */
  bool IsInitialized (void) const;

  /*
   * \brief Get the net-device source 
   * \param i the link requested
   * \returns Ptr to NOCNetDevice source for the 
   * specified link
   */
  Ptr<NOCNetDevice> GetSource (uint32_t i) const;

  /*
   * \brief Get the net-device destination
   * \param i the link requested
   * \returns Ptr to NOCNetDevice destination for 
   * the specified link
   */
  Ptr<NOCNetDevice> GetDestination (uint32_t i) const;

private:
  // Each point to point link has exactly two net devices
  static const int N_DEVICES = 2;

  Time          m_delay;
  int32_t       m_nDevices;

  /**
   * The trace source for the packet transmission animation events that the 
   * device can fire.
   * Arguments to the callback are the packet, transmitting
   * net device, receiving net device, transmission time and 
   * packet receipt time.
   *
   * @see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet>, // Packet being transmitted
                 Ptr<NetDevice>,    // Transmitting NetDevice
                 Ptr<NetDevice>,    // Receiving NetDevice
                 Time,              // Amount of time to transmit the pkt
                 Time               // Last bit receive time (relative to now)
                 > m_txrxNOC;

  enum WireState
  {
    INITIALIZING,
    IDLE,
    TRANSMITTING,
    PROPAGATING
  };
  
  
  class Link
  {
public:
    Link() : m_state (INITIALIZING), m_src (0), m_dst (0) {}
    WireState                  m_state;
    Ptr<NOCNetDevice> m_src;
    Ptr<NOCNetDevice> m_dst;
  };

  Link    m_link[N_DEVICES];
  Link    m_signal_link[N_DEVICES];
};

} // namespace ns3

#endif /* POINT_TO_POINT_CHANNEL_H */
