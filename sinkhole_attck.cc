#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/aodv-module.h"
#include "ns3/energy-module.h"
#include "ns3/applications-module.h"

#include <fstream>
#include <iostream>
#include <iomanip> // For std::setprecision and std::fixed

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SinkholeAttackExample");

uint32_t totalPacketsSent = 0;
uint32_t totalPacketsReceived = 0;

// Energy logging variables
std::ofstream energyLogFile;
Time energyLogInterval = Seconds(5); // Adjusted logging interval for more frequent updates

// Packet sent callback
void PacketSentCallback(Ptr<const Packet> packet) {
  totalPacketsSent++;
  NS_LOG_UNCOND("Packet sent: " << packet->GetUid());
}

// Packet received callback
void PacketReceivedCallback(Ptr<const Packet> packet, const Address &address) {
  totalPacketsReceived++;
  NS_LOG_UNCOND("Packet received: " << packet->GetUid());
}

// Function to print node energy
void PrintNodeEnergy(Ptr<Node> node, Ptr<BasicEnergySource> basicSourcePtr, Time simTime) {
  double remainingEnergy = basicSourcePtr->GetRemainingEnergy();
  NS_LOG_UNCOND("Node " << node->GetId() << " Remaining energy: " << remainingEnergy << " J");

  // Write to energy log file
  energyLogFile << std::fixed << std::setprecision(3) << Simulator::Now().GetSeconds() << ","
                << node->GetId() << "," << remainingEnergy << "\n";

  // Schedule next energy print event
  Simulator::Schedule(energyLogInterval, &PrintNodeEnergy, node, basicSourcePtr, simTime);
}

// Functor object to update node color in animation
class ColorUpdater {
public:
  ColorUpdater(AnimationInterface *anim, Ptr<Node> node, uint8_t r, uint8_t g, uint8_t b)
    : m_anim(anim), m_node(node), m_r(r), m_g(g), m_b(b) {}

  void operator()() {
    m_anim->UpdateNodeColor(m_node, m_r, m_g, m_b);
  }

private:
  AnimationInterface *m_anim;
  Ptr<Node> m_node;
  uint8_t m_r, m_g, m_b;
};

// Function to highlight packet transmission
void HighlightPacketTransmission(Ptr<Node> fromNode, Ptr<Node> toNode, AnimationInterface &anim, Time duration) {
  if (!fromNode || !toNode) {
    std::cerr << "Invalid nodes!" << std::endl;
    return;
  }
  anim.UpdateNodeColor(fromNode, 255, 255, 0); // Yellow for sender
  anim.UpdateNodeColor(toNode, 255, 165, 0); // Orange for receiver
  Simulator::Schedule(duration, ColorUpdater(&anim, fromNode, 0, 255, 0)); // Green after duration
  Simulator::Schedule(duration, ColorUpdater(&anim, toNode, 0, 255, 0)); // Green after duration
}

int main(int argc, char *argv[]) {
  // Enable logging
  LogComponentEnable("SinkholeAttackExample", LOG_LEVEL_INFO);

  // Create nodes
  NodeContainer nodes;
  nodes.Create(22); // 20 regular nodes + 1 start node + 1 sink node

  // Set up physical layer
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  // Set up wireless channel
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  Ptr<YansWifiChannel> channel = wifiChannel.Create();
  wifiPhy.SetChannel(channel);

  // Set up MAC layer
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  // Install wireless devices
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

  // Set up mobility - Arrange nodes in a 5x5 grid mesh structure
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", DoubleValue(0.0),
                                "MinY", DoubleValue(0.0),
                                "DeltaX", DoubleValue(100.0),
                                "DeltaY", DoubleValue(100.0),
                                "GridWidth", UintegerValue(5),
                                "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  // Configure network layer
  InternetStackHelper internet;
  AodvHelper aodv;
  internet.SetRoutingHelper(aodv); // Use AODV routing
  internet.Install(nodes);

  // Assign IP addresses
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  // Set up energy model
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(50.0)); // Initial energy in Joules
  EnergySourceContainer sources = basicSourceHelper.Install(nodes);

  // Set higher power consumption parameters
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(1.5)); // Transmission current in Amperes
  radioEnergyHelper.Set("RxCurrentA", DoubleValue(1.3)); // Reception current in Amperes
  radioEnergyHelper.Set("IdleCurrentA", DoubleValue(0.03)); // Idle current in Amperes
  radioEnergyHelper.Set("SleepCurrentA", DoubleValue(0.0001));

  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(devices, sources);

  // Open energy log file
  energyLogFile.open("energy_log.csv");
  energyLogFile << "Time (s),Node ID,Remaining Energy (J)\n";

  // Create a trace file for visualization
  AnimationInterface anim("Sinkhole_attack_example.xml");

  // Set up Sinkhole attack
  Ptr<Node> SinkholeNode1 = nodes.Get(2); // Sinkhole Node 1
  Ptr<Node> SinkholeNode2 = nodes.Get(7); // Sinkhole Node 2
  anim.UpdateNodeColor(SinkholeNode1, 0, 255, 0); // Red node (Sinkhole Node 1)
  anim.UpdateNodeColor(SinkholeNode2, 255, 0, 0); // Red node (Sinkhole Node 2)

  // Color remaining nodes differently (non-attacker nodes)
  for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    if (nodes.Get(i) != SinkholeNode1 && nodes.Get(i) != SinkholeNode2) {
      anim.UpdateNodeColor(nodes.Get(i), 0, 255, 0); // Green nodes (Regular nodes)
    }
  }

  // Set up start and sink nodes
  Ptr<Node> startNode = nodes.Get(20);
  Ptr<Node> sinkNode = nodes.Get(21);
  anim.UpdateNodeColor(startNode, 0, 0, 255); // Yellow node (Start Node)
  anim.UpdateNodeColor(sinkNode, 0, 255, 0); // Orange node (Sink Node)

  // Create a direct link between Sinkhole nodes to simulate the Sinkhole attack
  NodeContainer SinkholeNodes(SinkholeNode1, SinkholeNode2);
  NetDeviceContainer SinkholeDevices = wifi.Install(wifiPhy, wifiMac, SinkholeNodes);
  Ipv4InterfaceContainer SinkholeInterfaces = address.Assign(SinkholeDevices);

  // Set up TCP server and client
  uint16_t port = 50000; // Arbitrary port number
  PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(21), port));
  ApplicationContainer serverApps = sink.Install(sinkNode);
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(50.0));

  OnOffHelper onoff("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(21), port));
  onoff.SetAttribute("PacketSize", UintegerValue(1024));
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer clientApps = onoff.Install(startNode);
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(50.0));

  // Attach a callback to the server
  serverApps.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&PacketReceivedCallback));

  // Schedule node energy print
  for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource>(sources.Get(i));
    Simulator::Schedule(energyLogInterval, &PrintNodeEnergy, nodes.Get(i), basicSourcePtr, Seconds(50.0));
  }

  // Highlight packet transmission
  Simulator::Schedule(Seconds(2.0), &HighlightPacketTransmission, startNode, sinkNode, std::ref(anim), Seconds(1.0));

  // Configure NetAnim
  anim.SetMaxPktsPerTraceFile(500000); // Increase max packets per trace file if needed
  anim.EnablePacketMetadata(true); // Enable packet level metadata output
  anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(50)); // Enable IPv4 protocol counters

  // Start simulation
  Simulator::Stop(Seconds(50.0)); // Stop simulation after 50 seconds
  Simulator::Run();

  // Close energy log file
  energyLogFile.close();

  // Print results
  NS_LOG_UNCOND("Total Packets Sent: " << totalPacketsSent);
  NS_LOG_UNCOND("Total Packets Received: " << totalPacketsReceived);

  Simulator::Destroy();
  return 0;
}
