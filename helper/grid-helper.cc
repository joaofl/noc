/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/usn-net-device.h"
#include "ns3/usn-channel.h"
#include "ns3/usn-remote-channel.h"
#include "ns3/queue.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"
#include "src/core/model/object-base.h"

#include "ns3/trace-helper.h"
#include "grid-helper.h"

NS_LOG_COMPONENT_DEFINE ("GridHelper");

namespace ns3 {

GridHelper::GridHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::USNNetDevice");
  m_channelFactory.SetTypeId ("ns3::USNChannel");
  m_remoteChannelFactory.SetTypeId ("ns3::USNRemoteChannel");
}

void 
GridHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void
 GridHelper::SetNetworkAttribute (std::string name, const uint32_t value)
{
    if (name.compare("SizeX") == 0) //Is the same
    {
        m_sizeX = value;
    }
    else if (name.compare("SizeY") == 0) //Is the same
    {
        m_sizeY = value;
    }
    else if (name.compare("ConnectionWidth") == 0) //Is the same
    {
        m_connectionWidth = value;
    }
}

//void
//GridHelper::InstallApplication()
//{
//    //**************** Application Installation ******************
//
//    UNSInputData my_input_data;
//    if ( my_input_data.LoadFromFile(dir_input + "input-data.s.csv")  == 0){
//        cout << "Error loading the input data file at " << dir_input << "input-data.s.csv";
//        return -1;
//    }
//        
//        
//        //Load one matrix of one snapshot sensors data;
//
//
//    ApplicationContainer my_usn_sink_app_container;
//    ApplicationContainer my_usn_app_container;
//    ApplicationContainer my_usn_switch_container;
//    ApplicationContainer my_usn_sensor_container;
//
//    for (uint32_t i = 0; i < n_nodes; i++) {
//        Ptr<USNApp> my_usn_app = CreateObject<USNApp> ();
//        Ptr<USNSwitch> my_usn_switch = CreateObject<USNSwitch> ();
//        Ptr<USNSensor> my_usn_sensor = CreateObject<USNSensor> ();
//
//        uint32_t x = i % m_sizeX;
//        uint32_t y = floor(i / m_sizeX);
//
//
//        //Setup app
//        my_usn_app->IsSink = false;
//        my_usn_app->MaxHops = size_neighborhood;
//        //my_usn_switch->SetStartTime(Seconds(0));
//        my_usn_app->SetStartTime(Seconds(0));
//        my_usn_app->SamplingCycles = sampling_cycles; // at least 2, since the first is sacrificed to
//        my_usn_app->SamplingPeriod = sampling_period; // at least 2, since the first is sacrificed to
//        my_usn_app->OperationalMode = 255; //Not defined, since it is defined by the sink ND packet
//        // detect the neighborhood
//
//        //Setup Net Device's Callback
//        uint8_t n_devices = my_node_container.Get(i)->GetNDevices();
//
//        for (uint32_t j = 0; j < n_devices; j++) { //iterate to find which netdevice is the correct one
//            my_net_device = my_node_container.Get(i)->GetDevice(j)->GetObject<USNNetDevice>();
////            uint8_t n = my_net_device->GetUSNAddress();
//            ostringstream ss;
//            ss << i << "," << x << "," << y << "," << (int) j;
//            my_net_device->TraceConnect("MacRx", ss.str(), MakeCallback(&packet_received_netdevice_mac));
//            my_net_device->TraceConnect("MacTx", ss.str(), MakeCallback(&packet_sent_netdevice_mac));
//            
//            my_net_device->TraceConnect("PhyTxEnd", ss.str(), MakeCallback(&packet_exchanged_phy));
//            my_net_device->TraceConnect("PhyRxEnd", ss.str(), MakeCallback(&packet_exchanged_phy));
//        }
//
//        //Setup switch
//        ostringstream ss;
//        ss << i << "," << x << "," << y;
//
//        my_usn_switch->TraceConnect("SwitchRxTrace", ss.str(), MakeCallback(&packet_received_switch));
//        my_usn_switch->TraceConnect("SwitchTxTrace", ss.str(), MakeCallback(&packet_sent_switch));
//
//        //Setup sensor
//        my_usn_sensor->SensorPosition.x = x;
//        my_usn_sensor->SensorPosition.y = y;
//        my_usn_sensor->InputData = &my_input_data;
//
//
//        //Should be installed in this order!!!
//        my_node_container.Get(i)->AddApplication(my_usn_app);
//        my_node_container.Get(i)->AddApplication(my_usn_switch);
//        my_node_container.Get(i)->AddApplication(my_usn_sensor);
//
//
//
//        my_usn_app_container.Add(my_usn_app);
//        my_usn_switch_container.Add(my_usn_switch);
//        my_usn_sensor_container.Add(my_usn_sensor);
//    }
//
//    // Setting the sinks
//    //uint32_t nodes_n = m_sizeX * m_sizeY;
//
//    double delta_x = (double) m_sizeX / (2 * (double) sinks_n); 
//    // takes the individuals in the center of the network
//    //TODO: a way of manually setting it
//    
//    for (uint32_t i = 0; i < sinks_n; i++) {
//        uint32_t x = floor((i + 1) * 2 * delta_x - delta_x);
//        uint32_t y = floor((double) m_sizeY / 2);
//        uint32_t n = x + y * m_sizeX;
//        my_usn_app_container.Get(n)->GetObject<USNApp>()->IsSink = true;
//        my_usn_app_container.Get(n)->GetObject<USNApp>()->OperationalMode = operational_mode; //the sink should spread the operational mode to others
//        my_usn_sink_app_container.Add(my_usn_app_container.Get(n)); //container with the sinks only
//        ostringstream ss;
//        ss << n << "," << x << "," << y;
//        //        my_usn_switch_container.Get(n)->GetObject<USNSwitch>()->TraceConnect("SwitchRxTrace", "18,32", MakeCallback(&packets_received_sink));
//        //        my_usn_switch_container.Get(n)->GetObject<USNSwitch>()->TraceConnect("SwitchTxTrace", "58,33", MakeCallback(&packets_received_sink));
//        my_usn_switch_container.Get(n)->GetObject<USNSwitch>()->TraceConnect("SwitchRxTrace", ss.str(), MakeCallback(&packet_received_switch_sink));
//        my_usn_switch_container.Get(n)->GetObject<USNSwitch>()->TraceConnect("SwitchTxTrace", ss.str(), MakeCallback(&packet_received_switch_sink));
//    }
//
//    //************************************************************    
//}

void
GridHelper::InitializeNetwork()
{
    NetDeviceContainer my_net_device_container;
    Ptr<USNNetDevice> my_net_device;
    Mac48Address my_mac_address;

    NodeContainer my_node_container;
    
    my_node_container.Create(m_sizeX * m_sizeY);
    
//    uint32_t m_nodesCreated = my_node_container.GetN();
    
//    cout << "Network size = " << m_sizeX << " * " << m_sizeY << " = " << (unsigned int) n_nodes << endl;
    // Net devices Address reference
    //            4
    //         _______
    //        |       |
    //     3  |  SN   |  1
    //        |_______|
    //           
    //            2

    //*************** POSITIONING ***********************

    MobilityHelper nodes_mb; //mobility helper
    uint32_t interspace = 10;


    nodes_mb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    nodes_mb.SetPositionAllocator("ns3::GridPositionAllocator",
            "MinX", DoubleValue(0.0),
            "MinY", DoubleValue(0.0),
            "DeltaX", DoubleValue(interspace),
            "DeltaY", DoubleValue(interspace),
            "GridWidth", UintegerValue(m_sizeX),
            "LayoutType", StringValue("RowFirst"));


    nodes_mb.Install(my_node_container);

    //********** Net Devices installation ***************


    uint32_t node_this;
    for (uint64_t y = 0; y < m_sizeY; y++) {
        for (uint64_t x = 0; x < m_sizeX; x++) {

            node_this = x + y * m_sizeX;

            if (x != m_sizeX - 1) { //connect to the node in front of it
                my_net_device_container = Install(my_node_container.Get(node_this), my_node_container.Get(node_this + 1));

                my_net_device = my_net_device_container.Get(0)->GetObject<USNNetDevice>();
                my_net_device->SetAddress(Mac48Address::Allocate()); //data port 1 - RIGHT
                my_net_device->SetUSNAddress(1);

                my_net_device = my_net_device_container.Get(1)->GetObject<USNNetDevice>();
                my_net_device->SetAddress(Mac48Address::Allocate()); //data port 3 - LEFT
                my_net_device->SetUSNAddress(3);

                my_net_device_container.Get(0)->Initialize();
                my_net_device_container.Get(1)->Initialize();
            }
            if (y != m_sizeY - 1) { //connect to the node bellow
                my_net_device_container = Install(my_node_container.Get(node_this), my_node_container.Get(node_this + m_sizeX));

                my_net_device = my_net_device_container.Get(0)->GetObject<USNNetDevice>();
                my_net_device->SetAddress(Mac48Address::Allocate()); //data port 2 - DOWN
                my_net_device->SetUSNAddress(2);

                my_net_device = my_net_device_container.Get(1)->GetObject<USNNetDevice>();
                my_net_device->SetAddress(Mac48Address::Allocate()); //data port 4 - UP
                my_net_device->SetUSNAddress(4);

                my_net_device_container.Get(0)->Initialize();
                my_net_device_container.Get(1)->Initialize();
            }


        }

    }
}

void 
GridHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
GridHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
  m_remoteChannelFactory.Set (n1, v1);
}

void 
GridHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type USNNetDevice.
  //
  Ptr<USNNetDevice> device = nd->GetObject<USNNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("GridHelper::EnablePcapInternal(): Device " << device << " not of type ns3::USNNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, 
                                                     PcapHelper::DLT_PPP);
  pcapHelper.HookDefaultSink<USNNetDevice> (device, "PromiscSniffer", file);
}

void 
GridHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type USNNetDevice.
  //
  Ptr<USNNetDevice> device = nd->GetObject<USNNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("GridHelper::EnableAsciiInternal(): Device " << device << 
                   " not of type ns3::USNNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to 
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create 
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy 
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      //
      // The MacRx trace source provides our "r" event.
      //
      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<USNNetDevice> (device, "MacRx", theStream);

      //
      // The "+", '-', and 'd' events are driven by trace sources actually in the
      // transmit queue.
      //
      Ptr<Queue> queue = device->GetQueue ();
      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue> (queue, "Enqueue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue> (queue, "Drop", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue> (queue, "Dequeue", theStream);

      // PhyRxDrop trace source for "d" event
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<USNNetDevice> (device, "PhyRxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to providd a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for 
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the 
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static 
  // functions that are always there waiting for just such a case.
  //
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  oss << "/NodeList/" << nd->GetNode ()->GetId () << "/DeviceList/" << deviceid << "/$ns3::USNNetDevice/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::USNNetDevice/TxQueue/Enqueue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::USNNetDevice/TxQueue/Dequeue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::USNNetDevice/TxQueue/Drop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::USNNetDevice/PhyRxDrop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer 
GridHelper::Install (NodeContainer c)
{
  NS_ASSERT (c.GetN () == 2);
  return Install (c.Get (0), c.Get (1));
}

NetDeviceContainer 
GridHelper::Install (Ptr<Node> a, Ptr<Node> b)
{
  NetDeviceContainer container;

  Ptr<USNNetDevice> devA = m_deviceFactory.Create<USNNetDevice> ();
  devA->SetAddress (Mac48Address::Allocate ());
  a->AddDevice (devA);
  Ptr<Queue> queueAp0 = m_queueFactory.Create<Queue> ();
  Ptr<Queue> queueAp1 = m_queueFactory.Create<Queue> ();
  devA->SetQueue (queueAp0, queueAp1);
  Ptr<USNNetDevice> devB = m_deviceFactory.Create<USNNetDevice> ();
  devB->SetAddress (Mac48Address::Allocate ());
  b->AddDevice (devB);
  Ptr<Queue> queueBp0 = m_queueFactory.Create<Queue> ();
  Ptr<Queue> queueBp1 = m_queueFactory.Create<Queue> ();
  devB->SetQueue (queueBp0, queueBp1);
  // If MPI is enabled, we need to see if both nodes have the same system id 
  // (rank), and the rank is the same as this instance.  If both are true, 
  //use a normal p2p channel, otherwise use a remote channel
  bool useNormalChannel = true;
  Ptr<USNChannel> channel = 0;

  if (MpiInterface::IsEnabled ())
    {
      uint32_t n1SystemId = a->GetSystemId ();
      uint32_t n2SystemId = b->GetSystemId ();
      uint32_t currSystemId = MpiInterface::GetSystemId ();
      if (n1SystemId != currSystemId || n2SystemId != currSystemId) 
        {
          useNormalChannel = false;
        }
    }
  if (useNormalChannel)
    {
      channel = m_channelFactory.Create<USNChannel> ();
    }
  else
    {
      channel = m_remoteChannelFactory.Create<USNRemoteChannel> ();
      Ptr<MpiReceiver> mpiRecA = CreateObject<MpiReceiver> ();
      Ptr<MpiReceiver> mpiRecB = CreateObject<MpiReceiver> ();
      mpiRecA->SetReceiveCallback (MakeCallback (&USNNetDevice::Receive, devA));
      mpiRecB->SetReceiveCallback (MakeCallback (&USNNetDevice::Receive, devB));
      devA->AggregateObject (mpiRecA);
      devB->AggregateObject (mpiRecB);
    }

  devA->Attach (channel);
  devB->Attach (channel);
  container.Add (devA);
  container.Add (devB);

  return container;
}

NetDeviceContainer 
GridHelper::Install (Ptr<Node> a, std::string bName)
{
  Ptr<Node> b = Names::Find<Node> (bName);
  return Install (a, b);
}

NetDeviceContainer 
GridHelper::Install (std::string aName, Ptr<Node> b)
{
  Ptr<Node> a = Names::Find<Node> (aName);
  return Install (a, b);
}

NetDeviceContainer 
GridHelper::Install (std::string aName, std::string bName)
{
  Ptr<Node> a = Names::Find<Node> (aName);
  Ptr<Node> b = Names::Find<Node> (bName);
  return Install (a, b);
}

} // namespace ns3

