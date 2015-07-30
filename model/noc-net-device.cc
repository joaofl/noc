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
 * Adapted by: João Loureiro <joflo@isep.ipp.pt>
 * 
 * 
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/noc-address.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "noc-net-device.h"
#include "noc-channel.h"

NS_LOG_COMPONENT_DEFINE("NOCNetDevice");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(NOCNetDevice)
    ;

    TypeId
    NOCNetDevice::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCNetDevice")
                .SetParent<NetDevice> ()
                .AddConstructor<NOCNetDevice> ()
                
                .AddAttribute("Mtu", "The MAC-level Maximum Transmission Unit",
                UintegerValue(DEFAULT_MTU),
                MakeUintegerAccessor(&NOCNetDevice::SetMtu,
                &NOCNetDevice::GetMtu),
                MakeUintegerChecker<uint16_t> ())
         
                .AddAttribute("SerialComm",
                "If the NetDevice uses serial or parallel communication",
                BooleanValue(true),
                MakeBooleanAccessor(&NOCNetDevice::m_serialComm),
                MakeBooleanChecker())
        
                .AddAttribute("Address",
                "The MAC address of this device.",
                NOCAddressValue(NOCAddress()),
                MakeNOCAddressAccessor(&NOCNetDevice::m_address),
                MakeNOCAddressChecker())
        
                .AddAttribute("DataRate",
                "The default data rate for point to point links",
                DataRateValue(DataRate("1Mb/s")),
                MakeDataRateAccessor(&NOCNetDevice::m_bps),
                MakeDataRateChecker())
                .AddAttribute("ReceiveErrorModel",
                "The receiver error model used to simulate packet loss",
                PointerValue(),
                MakePointerAccessor(&NOCNetDevice::m_receiveErrorModel),
                MakePointerChecker<ErrorModel> ())
        
                .AddAttribute("InterframeGap",
                "The time to wait between packet (frame) transmissions",
                TimeValue(Seconds(0.0)),
                MakeTimeAccessor(&NOCNetDevice::m_tInterframeGap),
                MakeTimeChecker())

//                .AddAttribute("ClockDrift",
//                "The delay caused by the clock drift between two hops",
//                TimeValue(Seconds(0.0)),
//                MakeTimeAccessor(&NOCNetDevice::m_clockDrift),
//                MakeTimeChecker())   
        
                .AddAttribute("ClockShift",
                "How much neighboring node's clock is shifted (value between 0 and 1 (0 and 100%))",
                DoubleValue(0),
                MakeDoubleAccessor(&NOCNetDevice::m_clockShift),
                MakeDoubleChecker<double_t>()) 
        
//                .AddAttribute("PacketDuration",
//                "The time the packet takes to be transmitted",
//                TimeValue(PicoSeconds(1500.0)),
//                MakeTimeAccessor(&NOCNetDevice::m_packetDuration),
//                MakeTimeChecker())
                //
                // Transmit queueing discipline for the device which includes its own set
                // of trace hooks.
                //
                .AddAttribute("TxQueue",
                "A queue to use as the transmit queue in the device.",
                PointerValue(),
                MakePointerAccessor(&NOCNetDevice::m_queue),
                MakePointerChecker<Queue> ())

                //
                // Trace sources at the "top" of the net device, where packets transition
                // to/from higher layers.
                //
                .AddTraceSource("MacTx",
                "Trace source indicating a packet has arrived for transmission by this device",
                MakeTraceSourceAccessor(&NOCNetDevice::m_macTxTrace),
                "ns3::NOCNetDevice::MacTxTrace")
                .AddTraceSource("MacTxDrop",
                "Trace source indicating a packet has been dropped by the device before transmission",
                MakeTraceSourceAccessor(&NOCNetDevice::m_macTxDropTrace),
                "ns3::NOCNetDevice::MacTxDropTrace")
                .AddTraceSource("MacPromiscRx",
                "A packet has been received by this device, has been passed up from the physical layer "
                "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                MakeTraceSourceAccessor(&NOCNetDevice::m_macPromiscRxTrace),
                "ns3::NOCNetDevice::MacPromiscRxTrace")
                .AddTraceSource("MacRx",
                "A packet has been received by this device, has been passed up from the physical layer "
                "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                MakeTraceSourceAccessor(&NOCNetDevice::m_macRxTrace),
                "ns3::NOCNetDevice::MacRxTrace")
#if 0
                // Not currently implemented for this device
                .AddTraceSource("MacRxDrop",
                "Trace source indicating a packet was dropped before being forwarded up the stack",
                MakeTraceSourceAccessor(&NOCNetDevice::m_macRxDropTrace))
#endif
                //
                // Trace sources at the "bottom" of the net device, where packets transition
                // to/from the channel.
                //
                .AddTraceSource("PhyTxBegin",
                "Trace source indicating a packet has begun transmitting over the channel",
                MakeTraceSourceAccessor(&NOCNetDevice::m_phyTxBeginTrace),
                "ns3::NOCNetDevice::PhyTxBegin")
                .AddTraceSource("PhyTxEnd",
                "Trace source indicating a packet has been completely transmitted over the channel",
                MakeTraceSourceAccessor(&NOCNetDevice::m_phyTxEndTrace),
                "ns3::NOCNetDevice::PhyTxEnd")
                .AddTraceSource("PhyTxDrop",
                "Trace source indicating a packet has been dropped by the device during transmission",
                MakeTraceSourceAccessor(&NOCNetDevice::m_phyTxDropTrace),
                "ns3::NOCNetDevice::PhyTxDrop")
#if 0
                // Not currently implemented for this device
                .AddTraceSource("PhyRxBegin",
                "Trace source indicating a packet has begun being received by the device",
                MakeTraceSourceAccessor(&NOCNetDevice::m_phyRxBeginTrace))
#endif
                .AddTraceSource("PhyRxEnd",
                "Trace source indicating a packet has been completely received by the device",
                MakeTraceSourceAccessor(&NOCNetDevice::m_phyRxEndTrace),
                "ns3::NOCNetDevice::PhyRxEnd")
        
                .AddTraceSource("PhyRxDrop",
                "Trace source indicating a packet has been dropped by the device during reception",
                MakeTraceSourceAccessor(&NOCNetDevice::m_phyRxDropTrace),
                "ns3::NOCNetDevice::PhyRxDrop")

                //
                // Trace sources designed to simulate a packet sniffer facility (tcpdump).
                // Note that there is really no difference between promiscuous and 
                // non-promiscuous traces in a point-to-point link.
                //
                .AddTraceSource("Sniffer",
                "Trace source simulating a non-promiscuous packet sniffer attached to the device",
                MakeTraceSourceAccessor(&NOCNetDevice::m_snifferTrace),
                "ns3::NOCNetDevice::Sniffer")
        
                .AddTraceSource("PromiscSniffer",
                "Trace source simulating a promiscuous packet sniffer attached to the device",
                MakeTraceSourceAccessor(&NOCNetDevice::m_promiscSnifferTrace),
                "ns3::NOCNetDevice::PromiscSniffer")
                ;
        return tid;
    }

    NOCNetDevice::NOCNetDevice()
    :
    m_txMachineState(READY),
    m_channel(0),
    m_linkUp(false),
//    m_wait(READY),
    m_currentPkt(0) {
        NS_LOG_FUNCTION(this);
        queue_size_prioritized = 0;
        queue_size = 0;
    }

    NOCNetDevice::~NOCNetDevice() {
        NS_LOG_FUNCTION_NOARGS();
    }

    void
    NOCNetDevice::DoDispose() {
        NS_LOG_FUNCTION_NOARGS();
        m_node = 0;
        m_channel = 0;
        m_receiveErrorModel = 0;
        m_currentPkt = 0;
        NetDevice::DoDispose();
    }

    void
    NOCNetDevice::SetDataRate(DataRate bps) {
        NS_LOG_FUNCTION_NOARGS();
        m_bps = bps;
    }

    void
    NOCNetDevice::SetInterframeGap(Time t) {
        NS_LOG_FUNCTION_NOARGS();
        m_tInterframeGap = t;
    }

    bool
    NOCNetDevice::TransmitStart(Ptr<Packet> p) {
        NS_LOG_FUNCTION(this << p);
        NS_LOG_LOGIC("UID is " << p->GetUid() << ")");

        //
        // This function is called to start the process of transmitting a packet.
        // We need to tell the channel that we've started wiggling the wire and
        // schedule an event that will be executed when the transmission is complete.
        //
        NS_ASSERT_MSG(m_txMachineState == READY, "Must be READY to transmit");
        m_txMachineState = BUSY;
        m_currentPkt = p;
        m_phyTxBeginTrace(m_currentPkt);

        //the time required to send a single bit
        Time oneBitTransmissionTime = PicoSeconds(m_bps.CalculateBitsTxTime(1));
        Time txTime;
        
        if (m_serialComm == true){
            txTime =  PicoSeconds(oneBitTransmissionTime.GetPicoSeconds() * p->GetSize() * 8);
            txTime += oneBitTransmissionTime * m_clockShift;
        }
        else{
//            in parallel, one packet takes one cycle to be transmitted, considering
//            that port and packet has both the same width.
            txTime = PicoSeconds(oneBitTransmissionTime.GetPicoSeconds() + 
                    oneBitTransmissionTime.GetPicoSeconds() * m_clockShift); 
        }
        
        Time txCompleteTime = txTime + m_tInterframeGap;
        
        NS_LOG_LOGIC("Schedule TransmitCompleteEvent in " << txCompleteTime.GetSeconds() << "sec");
        Simulator::Schedule(txCompleteTime, &NOCNetDevice::TransmitComplete, this);

        bool result = m_channel->TransmitStart(p, this, txTime);
        if (result == false) {
            m_phyTxDropTrace(p);
        }
        return result;
    }

    void 
    NOCNetDevice::RemoteTransmitStarted(void){
//        SetLocalWait(true);
        //Callback here to the switch
    }
    
//    bool 
//    NOCNetDevice::GetRemoteWait(void){
//        
//        return m_channel->GetRemoteWait(this);
////        bool w = false;
////        
////        return w;
//    }
//    bool
//    NOCNetDevice::GetLocalWait() {
//        return m_wait;
//    }
//
//
//    void 
//    NOCNetDevice::SetLocalWait(bool w){
//        m_wait = w;
//    }
    
    void
    NOCNetDevice::TransmitComplete(void) {
        NS_LOG_FUNCTION_NOARGS();

        //
        // This function is called to when we're all done transmitting a packet.
        // We try and pull another packet off of the transmit queue.  If the queue
        // is empty, we are done, otherwise we need to start transmitting the
        // next packet.
        //
        NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
        m_txMachineState = READY;

        NS_ASSERT_MSG(m_currentPkt != 0, "NOCNetDevice::TransmitComplete(): m_currentPkt zero");

        m_phyTxEndTrace(m_currentPkt);
        m_currentPkt = 0;


        Ptr<Packet> p = m_queue_prioritized->Dequeue();
        queue_size_prioritized = m_queue_prioritized->GetNPackets();
        if (p != 0) {
            //
            // There was packets on the high p queue, send them...
            //
            m_snifferTrace(p);
            m_promiscSnifferTrace(p);
            TransmitStart(p);
            return;
        }
        p = m_queue->Dequeue();
        queue_size = m_queue->GetNPackets();
        if (p != 0) {
            //
            // There was no packets on the high p, but on the lp queue, send them...
            //
            m_snifferTrace(p);
            m_promiscSnifferTrace(p);
            TransmitStart(p);
            return;
        }
    }

    bool
    NOCNetDevice::Attach(Ptr<NOCChannel> ch) {
        NS_LOG_FUNCTION(this << &ch);

        m_channel = ch;

        m_channel->Attach(this);

        //
        // This device is up whenever it is attached to a channel.  A better plan
        // would be to have the link come up when both devices are attached, but this
        // is not done for now.
        //
        NotifyLinkUp();
        return true;
    }

    void
    NOCNetDevice::SetQueue(Ptr<Queue> qp0, Ptr<Queue> qp1) {
        NS_LOG_FUNCTION(this << qp0);
        NS_LOG_FUNCTION(this << qp1);
        m_queue = qp0;
        m_queue_prioritized = qp1;
    }
    
    Ptr<Queue>
    NOCNetDevice::GetQueue(void) const {
        NS_LOG_FUNCTION_NOARGS();
        return m_queue;
    }

    void
    NOCNetDevice::SetReceiveErrorModel(Ptr<ErrorModel> em) {
        NS_LOG_FUNCTION(this << em);
        m_receiveErrorModel = em;
    }

    void
    NOCNetDevice::Receive(Ptr<Packet> packet) {
        NS_LOG_FUNCTION(this << packet);
        uint16_t protocol = 0;

        if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt(packet)) {
            // 
            // If we have an error model and it indicates that it is time to lose a
            // corrupted packet, don't forward this packet up, let it go.
            //
            m_phyRxDropTrace(packet);
        } else {
            // 
            // Hit the trace hooks.  All of these hooks are in the same place in this 
            // device becuase it is so simple, but this is not usually the case in 
            // more complicated devices.
            //
            m_snifferTrace(packet);
            m_promiscSnifferTrace(packet);
            m_phyRxEndTrace(packet);

            //
            // Strip off the point-to-point protocol header and forward this packet
            // up the protocol stack.  Since this is a simple point-to-point link,
            // there is no difference in what the promisc callback sees and what the
            // normal receive callback sees.
            //
            //ProcessHeader (packet->Copy(), protocol);

            if (!m_promiscCallback.IsNull()) {
                m_macPromiscRxTrace(packet);

                m_promiscCallback(this, packet, protocol, GetRemote(), GetAddress(), NetDevice::PACKET_HOST);
            }

            m_macRxTrace(packet);
            m_rxCallback(this, packet, protocol, GetRemote());
        }
    }

    void
    NOCNetDevice::NotifyLinkUp(void) {
        m_linkUp = true;
        m_linkChangeCallbacks();
    }

    void
    NOCNetDevice::SetIfIndex(const uint32_t index) {
        m_ifIndex = index;
    }

    uint32_t
    NOCNetDevice::GetIfIndex(void) const {
        return m_ifIndex;
    }

    Ptr<Channel>
    NOCNetDevice::GetChannel(void) const {
        return m_channel;
    }

    //
    // This is a point-to-point device, so we really don't need any kind of address
    // information.  However, the base class NetDevice wants us to define the
    // methods to get and set the address.  Rather than be rude and assert, we let
    // clients get and set the address, but simply ignore them.

    void
    NOCNetDevice::SetAddress(Address address) {
//        m_address = Mac48Address::ConvertFrom(address);
        m_address = NOCAddress::ConvertFrom(address);
    }

    Address
    NOCNetDevice::GetAddress(void) const {
        return m_address;
    }

    bool
    NOCNetDevice::IsLinkUp(void) const {
        return m_linkUp;
    }

    void
    NOCNetDevice::AddLinkChangeCallback(Callback<void> callback) {
        m_linkChangeCallbacks.ConnectWithoutContext(callback);
    }

//    //
//    // This is a point-to-point device, so every transmission is a broadcast to
//    // all of the devices on the network.
//    //
//
//    bool
//    NOCNetDevice::IsBroadcast(void) const {
//        return true;
//    }
//

    bool
    NOCNetDevice::IsBroadcast(void) const {
        return true;
    }

//    
//     We don't really need any addressing information since this is a 
//     point-to-point device.  The base class NetDevice wants us to return a
//     broadcast address, so we make up something reasonable.
//    
//
    Address
    NOCNetDevice::GetBroadcast(void) const {
        return NOCAddress();
    }

    bool
    NOCNetDevice::IsMulticast(void) const {
        return true;
    }

    Address
    NOCNetDevice::GetMulticast(Ipv4Address multicastGroup) const {
        return NOCAddress();
    }

    Address
    NOCNetDevice::GetMulticast(Ipv6Address addr) const {
        NS_LOG_FUNCTION(this << addr);
        return NOCAddress();
    }

    bool
    NOCNetDevice::IsPointToPoint(void) const {
        return true;
    }
//
    bool
    NOCNetDevice::IsBridge(void) const {
        return false;
    }

    bool
    NOCNetDevice::Send(
            Ptr<Packet> packet,
            const Address &dest,
            uint16_t protocolNumber) {
        return false;
    }

    bool
    NOCNetDevice::Send(Ptr<Packet> packet) {
        //if no priority have been specified, send it with the lowest one;
        return Send(packet, 0);
    }

    bool
    NOCNetDevice::Send(Ptr<Packet> packet, uint8_t priority) {
        //Address dest = Mac48Address("ff:ff:ff:ff:ff:ff");
        //uint16_t protocolNumber = 0X800;

        NS_LOG_FUNCTION_NOARGS();
        //NS_LOG_LOGIC ("p=" << packet << ", dest=" << &dest);
        NS_LOG_LOGIC("UID is " << packet->GetUid());

        //
        // If IsLinkUp() is false it means there is no channel to send any packet 
        // over so we just hit the drop trace on the packet and return an error.
        //
        if (IsLinkUp() == false) {
            m_macTxDropTrace(packet);
            return false;
        }
        //
        // If there's a transmission in progress, we enque the packet for later
        // transmission; otherwise we send it now.
        //
        if (m_txMachineState == READY) {

            //TODO: change queue to an array of queue, so multiple priorities
            //can be utilized
            if (priority > 0) {
                if (m_queue_prioritized->Enqueue(packet) == true) {
                    queue_size_prioritized = m_queue_prioritized->GetNPackets();
                    packet = m_queue_prioritized->Dequeue();
                    m_snifferTrace(packet);
                    m_promiscSnifferTrace(packet);
                    m_macTxTrace(packet);
                    
                    return TransmitStart(packet);
                } else {
                    // Enqueue may fail (overflow)
                    m_macTxDropTrace(packet);
                    return false;
                }
            }


            // 
            // Even if the transmitter is immediately available, we still enqueue and
            // dequeue the packet to hit the tracing hooks.
            //
            if (m_queue->Enqueue(packet) == true) {
                queue_size = m_queue->GetNPackets();
                packet = m_queue->Dequeue();
                m_snifferTrace(packet);
                m_promiscSnifferTrace(packet);
                return TransmitStart(packet);
            } else {
                // Enqueue may fail (overflow)
                m_macTxDropTrace(packet);
                return false;
            }
        } else {
            if (priority > 0){
                bool r = m_queue_prioritized->Enqueue(packet);
                queue_size_prioritized = m_queue_prioritized->GetNPackets();
                return r;
            }
            else{
                bool r = m_queue->Enqueue(packet);
                queue_size = m_queue->GetNPackets();
                return r;
            }
        }
    }

    bool
    NOCNetDevice::SendFrom(Ptr<Packet> packet,
            const Address &source,
            const Address &dest,
            uint16_t protocolNumber) {
        return false;
    }

    Ptr<Node>
    NOCNetDevice::GetNode(void) const {
        return m_node;
    }

    void
    NOCNetDevice::SetNode(Ptr<Node> node) {
        m_node = node;
    }

    bool
    NOCNetDevice::NeedsArp(void) const {
        return false;
    }

    void
    NOCNetDevice::SetReceiveCallback(NOCNetDevice::ReceiveCallback cb) {
        m_rxCallback = cb;
    }

    void
    NOCNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) {
        m_promiscCallback = cb;
    }

    bool
    NOCNetDevice::SupportsSendFrom(void) const {
        return false;
    }

    void
    NOCNetDevice::DoMpiReceive(Ptr<Packet> p) {
        Receive(p);
    }

    Address
    NOCNetDevice::GetRemote(void) const {
        NS_ASSERT(m_channel->GetNDevices() == 2);
        for (uint32_t i = 0; i < m_channel->GetNDevices(); ++i) {
            Ptr<NetDevice> tmp = m_channel->GetDevice(i);
            if (tmp != this) {
                return tmp->GetAddress();
            }
        }
        NS_ASSERT(false);
        // quiet compiler.
        return Address();
    }

    bool
    NOCNetDevice::SetMtu(uint16_t mtu) {
        NS_LOG_FUNCTION(this << mtu);
        m_mtu = mtu;
        return true;
    }

    uint16_t
    NOCNetDevice::GetMtu(void) const {
        NS_LOG_FUNCTION_NOARGS();
        return m_mtu;
    }


} // namespace ns3
