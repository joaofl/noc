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

NS_LOG_COMPONENT_DEFINE ("USNChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (USNChannel)
  ;

TypeId 
USNChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::USNChannel")
    .SetParent<Channel> ()
    .AddConstructor<USNChannel> ()
    .AddAttribute ("Delay", "Transmission delay through the channel",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&USNChannel::m_delay),
                   MakeTimeChecker ())
    .AddTraceSource ("TxRxUSN",
                     "Trace source indicating transmission of packet from the USNChannel, used by the Animation interface.",
                     MakeTraceSourceAccessor (&USNChannel::m_txrxUSN))
  ;
  return tid;
}

//
// By default, you get a channel that 
// has an "infitely" fast transmission speed and zero delay.
USNChannel::USNChannel()
  :
    Channel (),
    m_delay (Seconds (0.)),
    m_nDevices (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
USNChannel::Attach (Ptr<USNNetDevice> device)
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
USNChannel::TransmitStart (
  Ptr<Packet> p,
  Ptr<USNNetDevice> src,
  Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);

  uint32_t wire = src == m_link[0].m_src ? 0 : 1;

  Simulator::ScheduleWithContext (m_link[wire].m_dst->GetNode ()->GetId (),
                                  txTime + m_delay, &USNNetDevice::Receive,
                                  m_link[wire].m_dst, p);

  // Call the tx anim callback on the net device
  m_txrxUSN (p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
  return true;
}

bool
USNChannel::TransmitSignalStart (
  Ptr<Packet> p,
  Ptr<USNNetDevice> src,
  Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  NS_ASSERT (m_signal_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_signal_link[1].m_state != INITIALIZING);

  uint32_t wire = src == m_signal_link[0].m_src ? 0 : 1;

  Simulator::ScheduleWithContext (m_signal_link[wire].m_dst->GetNode ()->GetId (),
                                  txTime + m_delay, &USNNetDevice::ReceiveSignal,
                                  m_signal_link[wire].m_dst, p);

  // Call the tx anim callback on the net device
  m_txrxUSN (p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
  return true;
}

uint32_t 
USNChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_nDevices;
}

Ptr<USNNetDevice>
USNChannel::GetUSNDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (i < 2);
  return m_link[i].m_src;
}

Ptr<NetDevice>
USNChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetUSNDevice (i);
}

Time
USNChannel::GetDelay (void) const
{
  return m_delay;
}

Ptr<USNNetDevice>
USNChannel::GetSource (uint32_t i) const
{
  return m_link[i].m_src;
}

Ptr<USNNetDevice>
USNChannel::GetDestination (uint32_t i) const
{
  return m_link[i].m_dst;
}

bool
USNChannel::IsInitialized (void) const
{
  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);
  return true;
}

} // namespace ns3
