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
 */

#ifndef NOC_NET_DEVICE_H
#define NOC_NET_DEVICE_H

#include <cstring>
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"

namespace ns3 {

    class Queue;
    class NOCChannel;
    class ErrorModel;

    /**
     * \defgroup point-to-point Point-To-Point Network Device
     * This section documents the API of the ns-3 point-to-point module. For a generic functional description, please refer to the ns-3 manual.
     */

    /**
     * \ingroup point-to-point
     * \class NOCNetDevice
     * \brief A Device for a Point to Point Network Link.
     *
     * This NOCNetDevice class specializes the NetDevice abstract
     * base class.  Together with a NOCChannel (and a peer 
     * NOCNetDevice), the class models, with some level of 
     * abstraction, a generic point-to-point or serial link.
     * Key parameters or objects that can be specified for this device 
     * include a queue, data rate, and interframe transmission gap (the 
     * propagation delay is set in the NOCChannel).
     */
    class NOCNetDevice : public NetDevice {
    public:
        static TypeId GetTypeId(void);


        uint32_t PacketsReceived;
        uint32_t PacketsReceivedP1;
        uint32_t PacketsReceivedP2;
        uint32_t PacketsReceivedP3;

        uint32_t PacketsSent;
        uint32_t PacketsSentP1;
        uint32_t PacketsSentP2;
        uint32_t PacketsSentP3;
        
        uint32_t queue_size_prioritized;
        uint32_t queue_size;

        /**
         * Construct a NOCNetDevice
         *
         * This is the constructor for the NOCNetDevice.  It takes as a
         * parameter a pointer to the Node to which this device is connected, 
         * as well as an optional DataRate object.
         */
        NOCNetDevice();

        /**
         * Destroy a NOCNetDevice
         *
         * This is the destructor for the NOCNetDevice.
         */
        virtual ~NOCNetDevice();

        /**
         * Set the Data Rate used for transmission of packets.  The data rate is
         * set in the Attach () method from the corresponding field in the channel
         * to which the device is attached.  It can be overridden using this method.
         *
         * @see Attach ()
         * @param bps the data rate at which this object operates
         */
        void SetDataRate(DataRate bps);

        /**
         * Set the interframe gap used to separate packets.  The interframe gap
         * defines the minimum space required between packets sent by this device.
         *
         * @param t the interframe gap time
         */
        void SetInterframeGap(Time t);

        /**
         * Attach the device to a channel.
         *
         * @param ch Ptr to the channel to which this object is being attached.
         */
        bool Attach(Ptr<NOCChannel> ch);

        /**
         * Attach a queue to the NOCNetDevice.
         *
         * The NOCNetDevice "owns" a queue that implements a queueing 
         * method such as DropTail or RED.
         *
         * @see Queue
         * @see DropTailQueue
         * @param queue Ptr to the new queues. With low and high priorities.
         */
        void SetQueue(Ptr<Queue> queuep0, Ptr<Queue> queuep1);

        /**
         * Get a copy of the attached Queue.
         *
         * @returns Ptr to the queue.
         */
        Ptr<Queue> GetQueue(void) const;

        /**
         * Attach a receive ErrorModel to the NOCNetDevice.
         *
         * The NOCNetDevice may optionally include an ErrorModel in
         * the packet receive chain.
         *
         * @see ErrorModel
         * @param em Ptr to the ErrorModel.
         */
        void SetReceiveErrorModel(Ptr<ErrorModel> em);

        /**
         * Receive a packet from a connected NOCChannel.
         *
         * The NOCNetDevice receives packets from its connected channel
         * and forwards them up the protocol stack.  This is the public method
         * used by the channel to indicate that the last bit of a packet has 
         * arrived at the device.
         *
         * @see NOCChannel
         * @param p Ptr to the received packet.
         */
        void Receive(Ptr<Packet> p);
        
        void ReceiveSignal(Ptr<Packet> p);

        /**
         * Sends a packet without the need of informing destination and protocol n.
         *
         * @param p Ptr to the packet to be transmitted.
         */
        bool Send(Ptr<Packet> packet);
        
        /**
         * Sends a packet without the need of informing destination and protocol n.
         *
         * @param p Ptr to the packet to be transmitted.
         * @param pri Priority on transmitting the packet (default 0, 1 for prioritized transmissions)
         */
        bool Send(Ptr<Packet> packet, uint8_t priority);
        
        bool SendSignal(Ptr<Packet> packet);

        void SetNOCAddress(uint32_t);
        uint32_t GetNOCAddress(void);

        // The remaining methods are documented in ns3::NetDevice*

        virtual void SetIfIndex(const uint32_t index);
        virtual uint32_t GetIfIndex(void) const;

        virtual Ptr<Channel> GetChannel(void) const;

        virtual void SetAddress(Address address);
        virtual Address GetAddress(void) const;

        virtual bool SetMtu(const uint16_t mtu);
        virtual uint16_t GetMtu(void) const;

        virtual bool IsLinkUp(void) const;

        virtual void AddLinkChangeCallback(Callback<void> callback);

        virtual bool IsBroadcast(void) const;
        virtual Address GetBroadcast(void) const;

        virtual bool IsMulticast(void) const;
        virtual Address GetMulticast(Ipv4Address multicastGroup) const;

        virtual bool IsPointToPoint(void) const;
        virtual bool IsBridge(void) const;

        virtual bool Send(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
        
        virtual bool SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

        virtual Ptr<Node> GetNode(void) const;
        virtual void SetNode(Ptr<Node> node);

        virtual bool NeedsArp(void) const;

        virtual void SetReceiveCallback(NetDevice::ReceiveCallback cb);

        virtual void SetReceiveSignalCallback(NetDevice::ReceiveCallback cb);
        
        virtual Address GetMulticast(Ipv6Address addr) const;

        virtual void SetPromiscReceiveCallback(PromiscReceiveCallback cb);
        virtual bool SupportsSendFrom(void) const;

    protected:
        void DoMpiReceive(Ptr<Packet> p);

    private:

        NOCNetDevice& operator =(const NOCNetDevice &);
        NOCNetDevice(const NOCNetDevice &);

        virtual void DoDispose(void);

    private:

        /**
         * \returns the address of the remote device connected to this device
         * through the point to point channel.
         */
        Address GetRemote(void) const;

        /**
         * Adds the necessary headers and trailers to a packet of data in order to
         * respect the protocol implemented by the agent.
         * \param p packet
         * \param protocolNumber protocol number
         */
        void AddHeader(Ptr<Packet> p, uint16_t protocolNumber);

        /**
         * Removes, from a packet of data, all headers and trailers that
         * relate to the protocol implemented by the agent
         * \param p Packet whose headers need to be processed
         * \param param An integer parameter that can be set by the function
         * \return Returns true if the packet should be forwarded up the
         * protocol stack.
         */
        bool ProcessHeader(Ptr<Packet> p, uint16_t& param);

        /**
         * Start Sending a Packet Down the Wire.
         *
         * The TransmitStart method is the method that is used internally in the
         * NOCNetDevice to begin the process of sending a packet out on
         * the channel.  The corresponding method is called on the channel to let
         * it know that the physical device this class represents has virtually
         * started sending signals.  An event is scheduled for the time at which
         * the bits have been completely transmitted.
         *
         * @see NOCChannel::TransmitStart ()
         * @see TransmitCompleteEvent ()
         * @param p a reference to the packet to send
         * @returns true if success, false on failure
         */
        bool TransmitStart(Ptr<Packet> p);

        /**
         * Stop Sending a Packet Down the Wire and Begin the Interframe Gap.
         *
         * The TransmitComplete method is used internally to finish the process
         * of sending a packet out on the channel.
         */
        void TransmitComplete(void);

        void NotifyLinkUp(void);

        /**
         * Enumeration of the states of the transmit machine of the net device.
         */
        enum TxMachineState {
            READY, /**< The transmitter is ready to begin transmission of a packet */
            BUSY /**< The transmitter is busy transmitting a packet */
        };
        /**
         * The state of the Net Device transmit state machine.
         * @see TxMachineState
         */
        TxMachineState m_txMachineState;
        
        TxMachineState m_txSignalMachineState;

        /**
         * The data rate that the Net Device uses to simulate packet transmission
         * timing.
         * @see class DataRate
         */
        DataRate m_bps;

        /**
         * The interframe gap that the Net Device uses to throttle packet
         * transmission
         * @see class Time
         */
        Time m_tInterframeGap;

        /**
         * The NOCChannel to which this NOCNetDevice has been
         * attached.
         * @see class NOCChannel
         */
        Ptr<NOCChannel> m_channel;

        /**
         * The Queue which this NOCNetDevice uses as a packet source.
         * Management of this Queue has been delegated to the NOCNetDevice
         * and it has the responsibility for deletion.
         * @see class Queue
         * @see class DropTailQueue
         */
        Ptr<Queue> m_queue;
        
        Ptr<Queue> m_queue_prioritized;
        
        Ptr<Queue> m_signal_queue;

        /**
         * Error model for receive packet events
         */
        Ptr<ErrorModel> m_receiveErrorModel;

        /**
         * The trace source fired when packets come into the "top" of the device
         * at the L3/L2 transition, before being queued for transmission.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_macTxTrace;

        /**
         * The trace source fired when packets coming into the "top" of the device
         * at the L3/L2 transition are dropped before being queued for transmission.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

        /**
         * The trace source fired for packets successfully received by the device
         * immediately before being forwarded up to higher layers (at the L2/L3 
         * transition).  This is a promiscuous trace (which doesn't mean a lot here
         * in the point-to-point device).
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

        /**
         * The trace source fired for packets successfully received by the device
         * immediately before being forwarded up to higher layers (at the L2/L3 
         * transition).  This is a non-promiscuous trace (which doesn't mean a lot 
         * here in the point-to-point device).
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_macRxTrace;

        /**
         * The trace source fired for packets successfully received by the device
         * but are dropped before being forwarded up to higher layers (at the L2/L3 
         * transition).
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

        /**
         * The trace source fired when a packet begins the transmission process on
         * the medium.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_phyTxBeginTrace;

        /**
         * The trace source fired when a packet ends the transmission process on
         * the medium.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_phyTxEndTrace;

        /**
         * The trace source fired when the phy layer drops a packet before it tries
         * to transmit it.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_phyTxDropTrace;

        /**
         * The trace source fired when a packet begins the reception process from
         * the medium -- when the simulated first bit(s) arrive.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_phyRxBeginTrace;

        /**
         * The trace source fired when a packet ends the reception process from
         * the medium.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_phyRxEndTrace;

        /**
         * The trace source fired when the phy layer drops a packet it has received.
         * This happens if the receiver is not enabled or the error model is active
         * and indicates that the packet is corrupt.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;

        /**
         * A trace source that emulates a non-promiscuous protocol sniffer connected 
         * to the device.  Unlike your average everyday sniffer, this trace source 
         * will not fire on PACKET_OTHERHOST events.
         *
         * On the transmit size, this trace hook will fire after a packet is dequeued
         * from the device queue for transmission.  In Linux, for example, this would
         * correspond to the point just before a device hard_start_xmit where 
         * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
         * ETH_P_ALL handlers.
         *
         * On the receive side, this trace hook will fire when a packet is received,
         * just before the receive callback is executed.  In Linux, for example, 
         * this would correspond to the point at which the packet is dispatched to 
         * packet sniffers in netif_receive_skb.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_snifferTrace;

        /**
         * A trace source that emulates a promiscuous mode protocol sniffer connected
         * to the device.  This trace source fire on packets destined for any host
         * just like your average everyday packet sniffer.
         *
         * On the transmit size, this trace hook will fire after a packet is dequeued
         * from the device queue for transmission.  In Linux, for example, this would
         * correspond to the point just before a device hard_start_xmit where 
         * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET 
         * ETH_P_ALL handlers.
         *
         * On the receive side, this trace hook will fire when a packet is received,
         * just before the receive callback is executed.  In Linux, for example, 
         * this would correspond to the point at which the packet is dispatched to 
         * packet sniffers in netif_receive_skb.
         *
         * \see class CallBackTraceSource
         */
        TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

        Ptr<Node> m_node;
        Mac48Address m_address;
        NetDevice::ReceiveCallback m_rxCallback;
        NetDevice::PromiscReceiveCallback m_promiscCallback;
        
        NetDevice::ReceiveCallback m_rxSignalCallback;
        
        uint32_t m_ifIndex;
        bool m_linkUp;
        TracedCallback<> m_linkChangeCallbacks;

        static const uint16_t DEFAULT_MTU = 1500;

        /**
         * The Maximum Transmission Unit.  This corresponds to the maximum 
         * number of bytes that can be transmitted as seen from higher layers.
         * This corresponds to the 1500 byte MTU size often seen on IP over 
         * Ethernet.
         */
        uint32_t m_mtu;

        uint32_t m_noc_address;

        Ptr<Packet> m_currentPkt;

    };

} // namespace ns3

#endif /* POINT_TO_POINT_NET_DEVICE_H */
