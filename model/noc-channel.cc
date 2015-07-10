/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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
 * 
 * 
 */

#include "noc-channel.h"
#include "noc-net-device.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/noc-net-device.h"

NS_LOG_COMPONENT_DEFINE ("NOCChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NOCChannel)
  ;

TypeId 
NOCChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NOCChannel")
    .SetParent<Channel> ()
    .AddConstructor<NOCChannel> ()
    .AddAttribute ("Delay", "Transmission delay through the channel",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&NOCChannel::m_delay),
                   MakeTimeChecker ())
    .AddTraceSource ("TxRxNOC",
                     "Trace source indicating transmission of packet from the NOCChannel, used by the Animation interface.",
                     MakeTraceSourceAccessor (&NOCChannel::m_txrxNOC))
  ;
  return tid;
}

//
// By default, you get a channel that 
// has an "infitely" fast transmission speed and zero delay.
NOCChannel::NOCChannel()
  :
    Channel (),
    m_delay (Seconds (0.)),
    m_nDevices (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NOCChannel::Attach (Ptr<NOCNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT_MSG (m_nDevices < N_DEVICES, "Only two devices permitted");
  NS_ASSERT (device != 0);

  m_link[m_nDevices++].m_src = device;
//
// If we have both devices connected to the channel, then finish introducing
// the two halves and set the links to IDLE.
//
  if (m_nDevices == N_DEVICES)
    {
      m_link[0].m_dst = m_link[1].m_src;
      m_link[1].m_dst = m_link[0].m_src;
      m_link[0].m_state = IDLE;
      m_link[1].m_state = IDLE;
    }
}

bool
NOCChannel::TransmitStart (
  Ptr<Packet> p,
  Ptr<NOCNetDevice> src,
  Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);

  uint32_t wire = src == m_link[0].m_src ? 0 : 1;

  Simulator::ScheduleWithContext (m_link[wire].m_dst->GetNode ()->GetId (),
                                  txTime + m_delay, &NOCNetDevice::Receive,
                                  m_link[wire].m_dst, p);

  // Call the tx anim callback on the net device
  m_txrxNOC (p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
  return true;
}

//bool
//NOCChannel::TransmitSignalStart (
//  Ptr<Packet> p,
//  Ptr<NOCNetDevice> src,
//  Time txTime)
//{
//  NS_LOG_FUNCTION (this << p << src);
//  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");
//
//  NS_ASSERT (m_signal_link[0].m_state != INITIALIZING);
//  NS_ASSERT (m_signal_link[1].m_state != INITIALIZING);
//
//  uint32_t wire = src == m_signal_link[0].m_src ? 0 : 1;
//
//  Simulator::ScheduleWithContext (m_signal_link[wire].m_dst->GetNode ()->GetId (),
//                                  txTime + m_delay, &NOCNetDevice::ReceiveSignal,
//                                  m_signal_link[wire].m_dst, p);
//
//  // Call the tx anim callback on the net device
//  m_txrxNOC (p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
//  return true;
//}

uint32_t 
NOCChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_nDevices;
}

Ptr<NOCNetDevice>
NOCChannel::GetNOCDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (i < 2);
  return m_link[i].m_src;
}

Ptr<NetDevice>
NOCChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetNOCDevice (i);
}

Time
NOCChannel::GetDelay (void) const
{
  return m_delay;
}

Ptr<NOCNetDevice>
NOCChannel::GetSource (uint32_t i) const
{
  return m_link[i].m_src;
}

Ptr<NOCNetDevice>
NOCChannel::GetDestination (uint32_t i) const
{
  return m_link[i].m_dst;
}

bool
NOCChannel::IsInitialized (void) const
{
  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);
  return true;
}

} // namespace ns3
