/* 
 * File:   grid-helper.h
 * Author: joao
 *
 * Created on June 25, 2015, 3:24 PM
 */

#ifndef GRID_HELPER_H
#define	GRID_HELPER_H

#include <string>
#include <sstream>

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/deprecated.h"

#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

#include "ns3/trace-helper.h"
#include "ns3/mobility-module.h"

namespace ns3 {

class Queue;
class NetDevice;
class Node;
//class MobilityHelper;

/**
 * \brief Build a set of NOCNetDevice objects
 *
 * Normally we eschew multiple inheritance, however, the classes 
 * PcapUserHelperForDevice and AsciiTraceUserHelperForDevice are
 * "mixins".
 */
class GridHelper : public PcapHelperForDevice, public AsciiTraceHelperForDevice
{
public:
  /**
   * Create a NOCHelper to make life easier when creating point to
   * point networks.
   */
  GridHelper ();
  virtual ~GridHelper () {}
  
  

  /**
   * Each point to point net device must have a queue to pass packets through.
   * This method allows one to set the type of the queue that is automatically
   * created when the device is created and attached to a node.
   *
   * \param type the type of queue
   * \param n1 the name of the attribute to set on the queue
   * \param v1 the value of the attribute to set on the queue
   * \param n2 the name of the attribute to set on the queue
   * \param v2 the value of the attribute to set on the queue
   * \param n3 the name of the attribute to set on the queue
   * \param v3 the value of the attribute to set on the queue
   * \param n4 the name of the attribute to set on the queue
   * \param v4 the value of the attribute to set on the queue
   *
   * Set the type of queue to create and associated to each
   * NOCNetDevice created through NOCHelper::Install.
   */
  void SetQueue (std::string type,
                 std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                 std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                 std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                 std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::NOCNetDevice created
   * by NOCHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Channel created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::NOCChannel created
   * by NOCHelper::Install
   */
  void SetChannelAttribute (std::string name, const AttributeValue &value);

  
  /**
   * Set an attribute value of a network general parameter.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * These parameters are the required info for the helper to create a grid of nodes
   */
  void SetNetworkAttribute (std::string name, const uint32_t value);
  
  NodeContainer InitializeNetwork(void);
  
//  ApplicationContainer InstallApplication(NodeContainer nc, &Application);
  
  /**
   * \param c a set of nodes
   *
   * This method creates a ns3::NOCChannel with the
   * attributes configured by NOCHelper::SetChannelAttribute,
   * then, for each node in the input container, we create a 
   * ns3::NOCNetDevice with the requested attributes, 
   * a queue for this ns3::NetDevice, and associate the resulting 
   * ns3::NetDevice with the ns3::Node and ns3::NOCChannel.
   */
  NetDeviceContainer Install (NodeContainer c);

  /**
   * \param a first node
   * \param b second node
   *
   * Saves you from having to construct a temporary NodeContainer. 
   * Also, if MPI is enabled, for distributed simulations, 
   * appropriate remote point-to-point channels are created.
   */
  NetDeviceContainer Install (Ptr<Node> a, Ptr<Node> b);

  /**
   * \param a first node
   * \param bName name of second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NetDeviceContainer Install (Ptr<Node> a, std::string bName);

  /**
   * \param aName Name of first node
   * \param b second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NetDeviceContainer Install (std::string aName, Ptr<Node> b);

  /**
   * \param aNode Name of first node
   * \param bNode Name of second node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  NetDeviceContainer Install (std::string aNode, std::string bNode);

private:
  /**
   * \brief Enable pcap output the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (
    Ptr<OutputStreamWrapper> stream,
    std::string prefix,
    Ptr<NetDevice> nd,
    bool explicitFilename);

  ObjectFactory m_queueFactory;
  ObjectFactory m_channelFactory;
  ObjectFactory m_remoteChannelFactory;
  ObjectFactory m_deviceFactory;
  
//    cmd.AddValue("size_x", "Network size in the X axe", size_x);
//    cmd.AddValue("size_y", "Network size in the Y axe", size_y);
//    cmd.AddValue("operational_mode", "Operate by detecting events (=1) or for sampling all the network (=0)", operational_mode);
//    cmd.AddValue("size_neighborhood", "Neighborhood size", size_neighborhood);
//    cmd.AddValue("sinks_n", "Network size in the X axe", sinks_n);
//    cmd.AddValue("sampling_cycles", "The number of times each node should sample its sensor, and perform data exchange", sampling_cycles);
//    cmd.AddValue("sampling_period", "The period between each sensor sampling [us]", sampling_period);
//    cmd.AddValue("log_start_at_period", "The period from witch logging starts", log_start_at_period);
//    cmd.AddValue("baudrate", "The baudrate of the node's communication ports [kbps]", baudrate);
//    cmd.AddValue("io_data_dir", "Directory with the I/O simulation data", io_data_dir);
  
  uint32_t m_sizeX, m_sizeY, m_baudrate, m_channelCount, m_queueSize, m_nodesCreated;
  std::string m_topology; //"Grid" only for now
  
};

} // namespace ns3
#endif	/* GRID_HELPER_H */