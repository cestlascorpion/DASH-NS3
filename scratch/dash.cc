/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2016 Technische Universitaet Berlin
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
 */

// - TCP Stream server and user-defined number of clients connected with an AP
// - WiFi connection
// - Tracing of throughput, packet information is done in the client

#include <errno.h>
#include <ns3/buildings-module.h>
#include <ns3/config-store-module.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "ns3/applications-module.h"
#include "ns3/building-position-allocator.h"
#include "ns3/core-module.h"
#include "ns3/epc-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/object.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-module.h"

template <typename T>
std::string ToString(T val) {
  std::stringstream stream;
  stream << val;
  return stream.str();
}

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("TcpStreamExample");

static void ACEvent(Ptr<ConstantAccelerationMobilityModel> cvmm, Vector &speed,
                    Vector &delta) {
  cvmm->SetVelocityAndAcceleration(speed, delta);
}
static void COEvent(Ptr<ConstantVelocityMobilityModel> cvmm, Vector &speed) {
  cvmm->SetVelocity(speed);
}
int main(int argc, char *argv[]) {
  LogComponentEnable("TcpStreamExample", LOG_LEVEL_INFO);
  LogComponentEnable("TcpStreamClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("TcpStreamServerApplication", LOG_LEVEL_INFO);

  uint64_t segmentDuration = 1000000;  // ms==> 1s/segment
  uint32_t simulationId = 4;
  uint32_t numberOfClients = 1;
  uint32_t numberOfEnbs = 8;
  std::string adaptationAlgo = "segmentaware";
  std::string app_type = "Dash";  // Bulk sender | On-Off Sender | Dash
  double eNbTxPower = 20.0;       // 43
  int fading_model = 0;           // 0 for etu, 1 for eva
  int load = 0;                   // 0 for low load, 1 for high load
  int rlc_mode = 3;               // UM = 2; AM = 3
  int tx_mode = 2;
  int bandwidth = 25;
  std::string data_rate = "100Gbps";  // 100Gbps

  CommandLine cmd;
  cmd.Usage("Simulation of streaming with DASH.\n");
  cmd.AddValue("simulationId", "The simulation's index (for logging purposes)",
               simulationId);
  cmd.AddValue("numberOfClients", "The number of clients", numberOfClients);
  cmd.AddValue("numberOfEnbs", "The number of eNodeBs", numberOfEnbs);
  cmd.AddValue("segmentDuration",
               "The duration of a video segment in microseconds",
               segmentDuration);
  cmd.AddValue("adaptationAlgo",
               "The adaptation algorithm that the client uses for the "
               "simulation[festive | tobasco | sara | tomato | "
               "constbitrateW/H/T/WH/C...]",
               adaptationAlgo);
  cmd.AddValue("app_type", "source model[Bulk | OnOff | Dash][defalt:Dash]",
               app_type);
  cmd.AddValue("eNbTxPower", "Tx Power of eNB(dBm)[default:43dBm]", eNbTxPower);
  cmd.AddValue("fading_model",
               "fading mode[0 = ETU3 | 1 = EVA60][default:EVA60]",
               fading_model);
  cmd.AddValue("load", "load scenario:[0 = low | 1 = high][deault:0]", load);
  cmd.AddValue("rlc_mode", "RLC mode[2 = UM | 3 = AM][default:3]", rlc_mode);
  cmd.AddValue("tx_mode",
               "TX mode[0 = SISO | 1 = MIMO_mode1 | 2=MIMO_mode2][default:0]",
               tx_mode);
  cmd.AddValue("BandWidth", "Dl bandwidth and Ul bandwidth[default=100]",
               bandwidth);
  cmd.AddValue("DataRate",
               "DataRate for PointToPoint(pgw->remoteHost)[Default=100Gbps]",
               data_rate);
  cmd.Parse(argc, argv);

  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",
                     BooleanValue(false));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled",
                     BooleanValue(true));
  Config::SetDefault("ns3::LteEnbRrc::DefaultTransmissionMode",
                     UintegerValue(tx_mode));  // MIMO
  Config::SetDefault("ns3::LteEnbRrc::EpsBearerToRlcMapping",
                     EnumValue(rlc_mode));
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(eNbTxPower));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1446));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(524288));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(524288));

  ConfigStore input_config;
  input_config.ConfigureDefaults();
  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  lteHelper->SetAttribute("FadingModel",
                          StringValue("ns3::TraceFadingLossModel"));
  lteHelper->SetAttribute("PathlossModel",
                          StringValue("ns3::Cost231PropagationLossModel"));
  std::ifstream ifTraceFile;
  std::string fading_trace_path;

  if (simulationId == 1) fading_model = 0;
  if (simulationId == 2) fading_model = 0;
  if (simulationId == 3) fading_model = 1;
  if (simulationId == 4) fading_model = 1;

  if (fading_model == 0)
    fading_trace_path =
        "../../src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad";
  if (fading_model == 1)
    fading_trace_path =
        "../../src/lte/model/fading-traces/fading_trace_EVA_60kmph.fad";

  ifTraceFile.open(fading_trace_path.c_str(), std::ifstream::in);

  if (ifTraceFile.good())
    lteHelper->SetFadingModelAttribute("TraceFilename",
                                       StringValue(fading_trace_path.c_str()));
  else
    lteHelper->SetFadingModelAttribute(
        "TraceFilename", StringValue(fading_trace_path.substr(6).c_str()));

  lteHelper->SetFadingModelAttribute("TraceLength", TimeValue(Seconds(10.0)));
  lteHelper->SetFadingModelAttribute("SamplesNum", UintegerValue(10000));
  lteHelper->SetFadingModelAttribute("WindowSize", TimeValue(Seconds(0.5)));
  lteHelper->SetFadingModelAttribute("RbNum", UintegerValue(bandwidth));
  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(bandwidth));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(bandwidth));
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

  Ptr<Node> pgw = epcHelper->GetPgwNode();
  std::cout << "pgw Id:  " << pgw->GetId() << std::endl;
  NodeContainer remote_host_container;
  remote_host_container.Create(1);
  Ptr<Node> remote_host = remote_host_container.Get(0);
  InternetStackHelper internet;
  internet.Install(remote_host_container);
  std::cout << "remoteHost Id:  " << remote_host->GetId() << std::endl;
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate(data_rate)));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.001)));
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remote_host);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remote_host_static_routing =
      ipv4RoutingHelper.GetStaticRouting(remote_host->GetObject<Ipv4>());
  remote_host_static_routing->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
                                                Ipv4Mask("255.0.0.0"), 1);

  NodeContainer eNb_nodes;
  NodeContainer ue_nodes;
  Ipv4InterfaceContainer ueIpIface;
  ue_nodes.Create(numberOfClients);
  eNb_nodes.Create(numberOfEnbs);
  MobilityHelper enbMobility;
  Ptr<ListPositionAllocator> positionAlloc_eNB =
      CreateObject<ListPositionAllocator>();

  positionAlloc_eNB->Add(Vector(0, 0, 0));     // eNB_0
  positionAlloc_eNB->Add(Vector(180, 0, 0));   // eNB_1
  positionAlloc_eNB->Add(Vector(360, 0, 0));   // eNB_2
  positionAlloc_eNB->Add(Vector(540, 0, 0));   // eNB_3
  positionAlloc_eNB->Add(Vector(720, 0, 0));   // eNB_4
  positionAlloc_eNB->Add(Vector(900, 0, 0));   // eNB_5
  positionAlloc_eNB->Add(Vector(1080, 0, 0));  // eNB_6
  positionAlloc_eNB->Add(Vector(1260, 0, 0));  // eNB_7

  enbMobility.SetPositionAllocator(positionAlloc_eNB);
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.Install(eNb_nodes);

  // create folder
  std::string dir = "mylogs/";
  std::string subdir = dir + adaptationAlgo + "/";
  std::string ssubdir = subdir + ToString(numberOfClients) + "/";

  const char *mylogsDir = (dir).c_str();
  mkdir(mylogsDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  const char *tobascoDir = (subdir).c_str();
  mkdir(tobascoDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  const char *logdir = (ssubdir).c_str();
  mkdir(logdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  std::ofstream clientPosLog;
  std::string clientPos =
      ssubdir + "sim" + ToString(simulationId) + "_" + "clientPos.txt";
  clientPosLog.open(clientPos.c_str());
  NS_ASSERT_MSG(clientPosLog.is_open(), "Couldn't open clientPosLog file");

  switch (simulationId) {
    case 1: {  //固定位置
      Ptr<ListPositionAllocator> positionAlloc =
          CreateObject<ListPositionAllocator>();
      Ptr<RandomDiscPositionAllocator> randPosAlloc =
          CreateObject<RandomDiscPositionAllocator>();
      for (uint i = 0; i < numberOfClients; i++) {
        Vector pos = Vector(30, 24, 0);
        positionAlloc->Add(pos);

        clientPosLog << ToString(pos.x) << ", " << ToString(pos.y) << ", "
                     << ToString(pos.z) << "\n";
        clientPosLog.flush();
      }
      MobilityHelper ueMobility_1;
      ueMobility_1.SetPositionAllocator(positionAlloc);
      ueMobility_1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      ueMobility_1.Install(ue_nodes);
      break;
    }
    case 2: {  //隨機走動
      MobilityHelper ueMobility_2;
      ueMobility_2.SetMobilityModel(
          "ns3::RandomWalk2dMobilityModel", "Mode", StringValue("Time"), "Time",
          StringValue("1s"), "Speed",
          StringValue("ns3::ConstantRandomVariable[Constant=0.83333]"),
          "Bounds", RectangleValue(Rectangle(70, 50, 50, 30)));
      for (uint i = 0; i < numberOfClients; i++) {
        ueMobility_2.Install(ue_nodes.Get(i));
      }
      break;
    }
    case 3: {  //步行穿小區
      MobilityHelper ueMobility_4;
      ueMobility_4.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
      ueMobility_4.SetPositionAllocator(
          "ns3::UniformDiscPositionAllocator", "X", DoubleValue(-90.0), "Y",
          DoubleValue(-10), "rho", DoubleValue(0));
      ueMobility_4.Install(ue_nodes);
      for (int64_t i = 0; i < ue_nodes.GetN(); i++) {
        Ptr<ConstantVelocityMobilityModel> cvmm =
            ue_nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        cvmm->SetVelocity(Vector(0.833, 0.0, 0.0));
        Simulator::Schedule(Seconds(216), &COEvent, cvmm,
                            Vector(-0.833, 0.0, 0.0));
      }
      break;
    }
    case 4: {  //車載穿小區
      MobilityHelper ueMobility_5;
      ueMobility_5.SetMobilityModel("ns3::ConstantAccelerationMobilityModel");
      ueMobility_5.SetPositionAllocator("ns3::UniformDiscPositionAllocator",
                                        "X", DoubleValue(-90.0), "Y",
                                        DoubleValue(0), "rho", DoubleValue(0));
      ueMobility_5.Install(ue_nodes);
      for (int64_t i = 0; i < ue_nodes.GetN(); i++) {
        Ptr<ConstantAccelerationMobilityModel> cvmm =
            ue_nodes.Get(i)->GetObject<ConstantAccelerationMobilityModel>();
        cvmm->SetVelocityAndAcceleration(Vector(6, 0, 0), Vector(0, 0, 0));
        Simulator::Schedule(Seconds(30.0), &ACEvent, cvmm, Vector(-6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(60.0), &ACEvent, cvmm, Vector(6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(90.0), &ACEvent, cvmm, Vector(-6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(120.0), &ACEvent, cvmm, Vector(6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(150.0), &ACEvent, cvmm, Vector(-6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(180.0), &ACEvent, cvmm, Vector(6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(210.0), &ACEvent, cvmm, Vector(-6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(240.0), &ACEvent, cvmm, Vector(6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(270.0), &ACEvent, cvmm, Vector(-6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(300.0), &ACEvent, cvmm, Vector(6.0, 0, 0),
                            Vector(0, 0, 0));
        Simulator::Schedule(Seconds(330.0), &ACEvent, cvmm, Vector(-6.0, 0, 0),
                            Vector(0, 0, 0));
      }
      break;
    }
  }

  NetDeviceContainer eNb_devs = lteHelper->InstallEnbDevice(eNb_nodes);
  NetDeviceContainer ue_devs = lteHelper->InstallUeDevice(ue_nodes);

  internet.Install(ue_nodes);
  ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ue_devs));

  lteHelper->AttachToClosestEnb(ue_devs, eNb_devs);

  for (uint32_t i = 0; i < ue_nodes.GetN(); i++) {
    Ptr<Node> uenode = ue_nodes.Get(i);
    Ptr<Ipv4StaticRouting> ue_static_routing =
        ipv4RoutingHelper.GetStaticRouting(uenode->GetObject<Ipv4>());
    ue_static_routing->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(),
                                       1);
  }
  lteHelper->EnableTraces();

  std::vector<std::pair<Ptr<Node>, std::string>> clients;
  for (NodeContainer::Iterator i = ue_nodes.Begin(); i != ue_nodes.End(); ++i) {
    std::pair<Ptr<Node>, std::string> client(*i, adaptationAlgo);
    clients.push_back(client);
  }
  if (app_type.compare("Dash") == 0) {
    const Ptr<PhyRxStatsCalculator> lte_phy_rx_stats =
        lteHelper->GetPhyRxStats();
    uint16_t port = 80;
    TcpStreamServerHelper serverHelper(port);
    ApplicationContainer serverApp =
        serverHelper.Install(remote_host_container.Get(0));
    serverApp.Start(Seconds(1.0));

    TcpStreamClientHelper clientHelper(internetIpIfaces.GetAddress(1), port,
                                       lte_phy_rx_stats);
    clientHelper.SetAttribute("SegmentDuration",
                              UintegerValue(segmentDuration));
    clientHelper.SetAttribute("NumberOfClients",
                              UintegerValue(numberOfClients));
    clientHelper.SetAttribute("SimulationId", UintegerValue(simulationId));

    ApplicationContainer clientApps = clientHelper.Install(clients);
    clientApps.Get(0)->SetStartTime(Seconds(2));
    clientApps.Get(0)->SetStopTime(Seconds(320));
    // clientApps.Get(1)->SetStartTime(Seconds(2));
    // clientApps.Get(1)->SetStopTime(Seconds(320));
    NS_LOG_INFO("Run Simulation.");
    NS_LOG_INFO("Sim:   " << simulationId
                          << "   Clients:   " << numberOfClients);
    Simulator::Stop(Seconds(321));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
  }
  return 0;
}