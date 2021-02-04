/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
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
 * Authors: Phani Kiran S V S <phanikiran.harithas@gmail.com>
 *          Nichit Bodhak Goel <nichit93@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"

#include <iostream>
#include <iomanip>
#include <map>

// RRED paper topology
// 
// n0  n1 ..... n50 (30 users and 20 attackers connect to r0 with p2p link)
//  |   |        |
//  ==============
//         |
//        r0 -------------------r1----------------------server
//    (RED queue)


using namespace ns3;

int main (int argc, char *argv[])
{
  uint32_t    nUser = 30;		// Paper value 30
  uint32_t	  nAttacker	= 20;	// Paper value 20
  uint32_t    maxPackets = 1;
  bool        modeBytes  = false;
  uint32_t    queueDiscLimitPackets = 50;
  double      minTh = 1;
  double      maxTh = 3;
  uint32_t    TCPpktSize = 1000;
  uint32_t    UDPpktSize = 50;
  std::string OnOffappDataRate = "0.25Mbps";
  std::string p2pBw = "10Mbps";
  std::string p2pDelay = "2ms";
  std::string queueDiscType = "RRED";
  uint16_t port = 5001;
  std::string bottleNeckLinkBw = "5Mbps";
  std::string bottleNeckLinkDelay = "6ms";
  int32_t	  shortRange = 10;
  int32_t	  upperBound = 10;
  int32_t  	  lowerBound = -1;

  CommandLine cmd;
  cmd.AddValue ("nUser",     "Number of legitimate user nodes", nUser);
  cmd.AddValue ("nAttacker",     "Number of attacker nodes", nAttacker);
  cmd.AddValue ("maxPackets","Max Packets allowed in the device queue", maxPackets);
  cmd.AddValue ("queueDiscLimitPackets","Max Packets allowed in the queue disc", queueDiscLimitPackets);
  cmd.AddValue ("queueDiscType", "Set Queue disc type to RED or RRED", queueDiscType);
  cmd.AddValue ("TCPpktSize", "Set bulk-send App TCP Packet Size", TCPpktSize);
  cmd.AddValue ("UDPpktSize", "Set OnOff App UDP Packet Size", UDPpktSize);
  cmd.AddValue ("OnOffappDataRate", "Set OnOff App DataRate", OnOffappDataRate);
  cmd.AddValue ("modeBytes", "Set Queue disc mode to Packets <false> or bytes <true>", modeBytes);

  cmd.AddValue ("redMinTh", "RED queue minimum threshold", minTh);
  cmd.AddValue ("redMaxTh", "RED queue maximum threshold", maxTh);
  cmd.AddValue ("shortRange", "RRED short range between packets, in MilliSeconds", shortRange);
  cmd.AddValue ("upperBound", "RRED upper bound for local indicators", upperBound);
  cmd.AddValue ("lowerBound", "RRED lower bound for local indicators", lowerBound);
  cmd.Parse (argc,argv);

  if ((queueDiscType != "RED") && (queueDiscType != "RRED"))
      {
        std::cout << "Invalid queue disc type: Use --queueDiscType=RED or --queueDiscType=RRED" << std::endl;
        exit (1);
      }

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (UDPpktSize));
  Config::SetDefault ("ns3::BulkSendApplication::SendSize", UintegerValue (TCPpktSize));

  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (OnOffappDataRate));
  Config::SetDefault ("ns3::QueueBase::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, maxPackets)));


  if (!modeBytes)
     {
       Config::SetDefault ("ns3::RedQueueDisc::MaxSize",
                           QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, queueDiscLimitPackets)));
     }
   else
     {
       Config::SetDefault ("ns3::RedQueueDisc::MaxSize",
                           QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, queueDiscLimitPackets * TCPpktSize)));
       minTh *= TCPpktSize;
       maxTh *= TCPpktSize;
     }

  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minTh));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (maxTh));
  Config::SetDefault ("ns3::RedQueueDisc::LinkBandwidth", StringValue (bottleNeckLinkBw));
  Config::SetDefault ("ns3::RedQueueDisc::LinkDelay", StringValue (bottleNeckLinkDelay));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (TCPpktSize));

  if (queueDiscType == "RRED")
    {
      // Turn on NLRED and set attributes
      Config::SetDefault ("ns3::RedQueueDisc::RRED", BooleanValue (true));
      Config::SetDefault ("ns3::RedQueueDisc::UpperBound", IntegerValue(upperBound));
      Config::SetDefault ("ns3::RedQueueDisc::LowerBound", IntegerValue(lowerBound));
      Config::SetDefault ("ns3::RedQueueDisc::ShortRange", TimeValue (MilliSeconds(shortRange)));
    }


  // Start building network topology
  // r1-server nodes
  NodeContainer r1serverNodes;
  r1serverNodes.Create (2);

  // Create the point-to-point link helper(r1-server)
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute  ("DataRate", StringValue (p2pBw));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (p2pDelay));

  // Create net devices on r1-server links
  NetDeviceContainer r1serverDevices;
  r1serverDevices = pointToPoint.Install(r1serverNodes);

  // Create p2p star layout with 50 spoke nodes+1 hub, devices also created by the helper
  PointToPointStarHelper pointToPointStar ((nUser+nAttacker), pointToPoint);
  pointToPoint.SetDeviceAttribute  ("DataRate", StringValue (p2pBw));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (p2pDelay));

  // Create net devices on the bottleNeck link
  NodeContainer r0r1Nodes;
  r0r1Nodes.Add(pointToPointStar.GetHub());
  r0r1Nodes.Add(r1serverNodes.Get(0));

  // Create the point-to-point link helpers(bottleNeck r0-r1)
  PointToPointHelper bottleNeckLink;
  bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckLinkBw));
  bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckLinkDelay));

  // Create net devices on r0-r1 links
  NetDeviceContainer r0r1Devices;
  r0r1Devices = bottleNeckLink.Install(r0r1Nodes);

  // Install Stack on all the nodes
  InternetStackHelper stack;
  pointToPointStar.InstallStack(stack);
  stack.Install(r1serverNodes.Get(0));
  stack.Install(r1serverNodes.Get(1));

  // Create traffic control
  TrafficControlHelper tchBottleneck;
  QueueDiscContainer queueDiscs;
  tchBottleneck.SetRootQueueDisc ("ns3::RedQueueDisc");
  tchBottleneck.Install (r0r1Devices.Get(1));
  queueDiscs = tchBottleneck.Install (r0r1Devices.Get(0));

  // Assign IP address to r1-server devices
  Ipv4AddressHelper r1serverAddress;
  r1serverAddress.SetBase ("10.1.1.0", "255.255.255.0", "0.0.0.1");
  Ipv4InterfaceContainer r1serverInterfaces = r1serverAddress.Assign (r1serverDevices);

  // Assign IP addresses to devices in p2p star
  pointToPointStar.AssignIpv4Addresses (Ipv4AddressHelper ("10.2.1.0", "255.255.255.0", "0.0.0.1"));

  // Assign IP addresses to r0-r1 devices
  Ipv4AddressHelper r0r1Address;
  r0r1Address.SetBase ("10.3.1.0", "255.255.255.0", "0.0.0.1");
  Ipv4InterfaceContainer r0r1Interfaces = r0r1Address.Assign (r0r1Devices);

  // Install packet sink application on the pointToPoint server node
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = packetSinkHelper.Install(r1serverNodes.Get (1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (30.0));

  // The remote address of the packet sink
  Address remoteAddress (InetSocketAddress (r1serverInterfaces.GetAddress(1), port));

  // Install bulk sned TCP app on all p2pStar user nodes
  BulkSendHelper bulkSendHelper ("ns3::TcpSocketFactory", remoteAddress);
  bulkSendHelper.SetAttribute ("SendSize", UintegerValue(TCPpktSize));
  bulkSendHelper.SetAttribute ("MaxBytes", UintegerValue(0));

  ApplicationContainer bulkSendApps;
  for (uint32_t i = 0; i < nUser; i++){ //node#0-29
	  bulkSendApps.Add (bulkSendHelper.Install(pointToPointStar.GetSpokeNode(i)));
  }

  // Create on-off UDP app on all p2p attacker nodes
  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", remoteAddress);
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onOffHelper.SetAttribute ("DataRate", StringValue (OnOffappDataRate)); // "0.25Mbps"
  onOffHelper.SetAttribute ("PacketSize", UintegerValue(UDPpktSize));
  onOffHelper.SetAttribute ("MaxBytes", UintegerValue(0));

  ApplicationContainer onOffApps;
  for (uint32_t i = nUser; i < nUser+nAttacker; i++){ //node#30-49
  	  onOffApps.Add (onOffHelper.Install(pointToPointStar.GetSpokeNode(i)));
  }

  // Application start and end time
  bulkSendApps.Start (Seconds (0.5)); // Start 0.5 second after sink
  bulkSendApps.Stop (Seconds (15.0)); // Stop before the sink
  onOffApps.Start (Seconds (1.0));
  onOffApps.Stop (Seconds (15.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  std::cout << "Running the simulation" << std::endl;
  Simulator::Run ();

  QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();

  //if (st.GetNDroppedPackets (RedQueueDisc::UNFORCED_DROP) == 0)
  //  {
  //    std::cout << "There should be some unforced drops" << std::endl;
  //    exit (1);
  //  }

   //if (st.GetNDroppedPackets (QueueDisc::INTERNAL_QUEUE_DROP) != 0)
   // {
   //   std::cout << "There should be zero drops due to queue full" << std::endl;
   //   exit (1);
   // }

  std::cout << "*** Stats from the bottleneck queue disc ***" << std::endl;
  std::cout << st << std::endl;
  std::cout << "Destroying the simulation" << std::endl;

  Simulator::Destroy ();
  return 0;
}
