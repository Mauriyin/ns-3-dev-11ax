/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University of Washington
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
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-phy-header.h"
#include "ns3/wifi-utils.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-threshold-channel-bonding-manager.h"
#include "ns3/waveform-generator.h"
#include "ns3/non-communicating-net-device.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBondingTest");

class BondingTestSpectrumWifiPhy : public SpectrumWifiPhy
{
public:
  using SpectrumWifiPhy::SpectrumWifiPhy;
  using SpectrumWifiPhy::GetBand;
  using SpectrumWifiPhy::ResetSpectrumModel;
  void ResetSpectrumModel (void) override;
};

void
BondingTestSpectrumWifiPhy::ResetSpectrumModel (void)
{
  uint16_t channelWidth = GetChannelWidth ();
  m_rxSpectrumModel = WifiSpectrumValueHelper::GetSpectrumModel (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth));
  UpdateInterferenceHelperBands ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief static channel bonding
 *
 * In this test, we have four 802.11n transmitters and four 802.11n receivers.
 * A BSS is composed of one transmitter and one receiver.
 *
 * The first BSS occupies channel 36 and a channel width of 20 MHz.
 * The second BSS operates on channel 40 with a channel width of 20 MHz.
 * Both BSS 3 and BSS 4 makes uses of channel bonding with a 40 MHz channel width
 * and operates on channel 38 (= 36 + 40). The only difference between them is that
 * BSS 3 has channel 36 as primary channel, whereas BSS 4 has channel 40 has primary channel.
 *
 */
class TestStaticChannelBonding : public TestCase
{
public:
  TestStaticChannelBonding ();
  virtual ~TestStaticChannelBonding ();

protected:
  virtual void DoSetup (void);
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss1; ///< RX Phy BSS #1
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss2; ///< RX Phy BSS #2
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss3; ///< RX Phy BSS #3
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss4; ///< RX Phy BSS #4
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss1; ///< TX Phy BSS #1
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss2; ///< TX Phy BSS #2
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss3; ///< TX Phy BSS #3
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss4; ///< TX Phy BSS #4

  double m_expectedSnrBss1;  ///< Expected SNR for RX Phy #1
  double m_expectedSnrBss2;  ///< Expected SNR for RX Phy #2
  double m_expectedSnrBss3;  ///< Expected SNR for RX Phy #3
  double m_expectedSnrBss4;  ///< Expected SNR for RX Phy #4
  bool m_initializedSnrBss1; ///< Flag whether expected SNR for BSS 1 has been set
  bool m_initializedSnrBss2; ///< Flag whether expected SNR for BSS 2 has been set
  bool m_initializedSnrBss3; ///< Flag whether expected SNR for BSS 3 has been set
  bool m_initializedSnrBss4; ///< Flag whether expected SNR for BSS 4 has been set

  bool m_receptionBss1; ///< Flag whether a reception occured for BSS 1
  bool m_receptionBss2; ///< Flag whether a reception occured for BSS 2
  bool m_receptionBss3; ///< Flag whether a reception occured for BSS 3
  bool m_receptionBss4; ///< Flag whether a reception occured for BSS 4

  bool m_phyPayloadReceivedSuccessBss1; ///< Flag whether the PHY payload has been successfully received by BSS 1
  bool m_phyPayloadReceivedSuccessBss2; ///< Flag whether the PHY payload has been successfully received by BSS 2
  bool m_phyPayloadReceivedSuccessBss3; ///< Flag whether the PHY payload has been successfully received by BSS 3
  bool m_phyPayloadReceivedSuccessBss4; ///< Flag whether the PHY payload has been successfully received by BSS 4

  /**
   * Send packet function
   * \param bss the BSS of the transmitter belongs to
   */
  void SendPacket (uint8_t bss);

  /**
   * Callback triggered when a packet is starting to be processed for reception
   * \param context the context
   * \param p the received packet
   * \param rxPowersW the received power per channel band in watts
   */
  void RxCallback (std::string context, Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW);

  /**
   * Callback triggered when a packet has been successfully received
   * \param context the context
   * \param p the received packet
   * \param snr the signal to noise ratio
   * \param mode the mode used for the transmission
   * \param preamble the preamble used for the transmission
   */
  void RxOkCallback (std::string context, Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble);

  /**
   * Callback triggered when a packet has been unsuccessfully received
   * \param context the context
   * \param p the packet
   * \param snr the signal to noise ratio
   */
  void RxErrorCallback (std::string context, Ptr<const Packet> p, double snr);

  /**
   * Set expected SNR ((in dB) for a given BSS before the test case is run
   * \param snr the expected signal to noise ratio in dB
   * \param bss the BSS number
   */
  void SetExpectedSnrForBss (double snr, uint8_t bss);
  /**
   * Verify packet reception once the test case is run
   * \param expectedReception whether the reception should have occured
   * \param bss the BSS number
   */
  void VerifyResultsForBss (bool expectedReception, bool expectedPhyPayloadSuccess, uint8_t bss);

  /**
   * Reset the results
   */
  void Reset (void);

private:
  virtual void DoRun (void);

  /**
   * Check the PHY state
   * \param expectedState the expected PHY state
   * \param bss the BSS number
   */
  void CheckPhyState (WifiPhyState expectedState, uint8_t bss);
  /**
   * Check the secondary channel status
   * \param expectedIdle flag whether the secondary channel is expected to be deemed IDLE
   * \param bss the BSS number
   */
  void CheckSecondaryChannelStatus (bool expectedIdle, uint8_t bss);
};

TestStaticChannelBonding::TestStaticChannelBonding ()
  : TestCase ("Static channel bonding test"),
    m_expectedSnrBss1 (0.0),
    m_expectedSnrBss2 (0.0),
    m_expectedSnrBss3 (0.0),
    m_expectedSnrBss4 (0.0),
    m_initializedSnrBss1 (false),
    m_initializedSnrBss2 (false),
    m_initializedSnrBss3 (false),
    m_initializedSnrBss4 (false),
    m_receptionBss1 (false),
    m_receptionBss2 (false),
    m_receptionBss3 (false),
    m_receptionBss4 (false),
    m_phyPayloadReceivedSuccessBss1 (false),
    m_phyPayloadReceivedSuccessBss2 (false),
    m_phyPayloadReceivedSuccessBss3 (false),
    m_phyPayloadReceivedSuccessBss4 (false)
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);
  //LogComponentEnable ("WifiSpectrumValueHelper", logLevel);
  //LogComponentEnable ("WifiPhy", logLevel);
  //LogComponentEnable ("SpectrumWifiPhy", logLevel);
  //LogComponentEnable ("InterferenceHelper", logLevel);
  //LogComponentEnable ("MultiModelSpectrumChannel", logLevel);
}

void
TestStaticChannelBonding::Reset (void)
{
  m_expectedSnrBss1 = 0.0;
  m_expectedSnrBss2 = 0.0;
  m_expectedSnrBss3 = 0.0;
  m_expectedSnrBss4 = 0.0;
  m_initializedSnrBss1 = false;
  m_initializedSnrBss2 = false;
  m_initializedSnrBss3 = false;
  m_initializedSnrBss4 = false;
  m_receptionBss1 = false;
  m_receptionBss2 = false;
  m_receptionBss3 = false;
  m_receptionBss4 = false;
  m_phyPayloadReceivedSuccessBss1 = false;
  m_phyPayloadReceivedSuccessBss2 = false;
  m_phyPayloadReceivedSuccessBss3 = false;
  m_phyPayloadReceivedSuccessBss4 = false;
}

void
TestStaticChannelBonding::SetExpectedSnrForBss (double snr, uint8_t bss)
{
  if (bss == 1)
    {
      m_expectedSnrBss1 = snr;
      m_initializedSnrBss1 = true;
    }
  else if (bss == 2)
    {
      m_expectedSnrBss2 = snr;
      m_initializedSnrBss2 = true;
    }
  else if (bss == 3)
    {
      m_expectedSnrBss3 = snr;
      m_initializedSnrBss3 = true;
    }
  else if (bss == 4)
    {
      m_expectedSnrBss4 = snr;
      m_initializedSnrBss4 = true;
    }
}

void
TestStaticChannelBonding::VerifyResultsForBss (bool expectedReception, bool expectedPhyPayloadSuccess, uint8_t bss)
{
  if (bss == 1)
    {
      NS_TEST_ASSERT_MSG_EQ (m_receptionBss1, expectedReception, "m_receptionBss1 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccessBss1, expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccessBss1 is not equal to expectedPhyPayloadSuccess");
    }
  else if (bss == 2)
    {
      NS_TEST_ASSERT_MSG_EQ (m_receptionBss2, expectedReception, "m_receptionBss2 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccessBss2, expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccessBss2 is not equal to expectedPhyPayloadSuccess");
    }
  else if (bss == 3)
    {
      NS_TEST_ASSERT_MSG_EQ (m_receptionBss3, expectedReception, "m_receptionBss3 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccessBss3, expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccessBss3 is not equal to expectedPhyPayloadSuccess");
    }
  else if (bss == 4)
    {
      NS_TEST_ASSERT_MSG_EQ (m_receptionBss4, expectedReception, "m_receptionBss4 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccessBss4, expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccessBss4 is not equal to expectedPhyPayloadSuccess");
    }
}

void
TestStaticChannelBonding::CheckPhyState (WifiPhyState expectedState, uint8_t bss)
{
  WifiPhyState currentState;
  PointerValue ptr;
  if (bss == 1)
    {
      m_rxPhyBss1->GetAttribute ("State", ptr);
    }
  else if (bss == 2)
    {
      m_rxPhyBss2->GetAttribute ("State", ptr);
    }
  else if (bss == 3)
    {
      m_rxPhyBss3->GetAttribute ("State", ptr);
    }
  else if (bss == 4)
    {
      m_rxPhyBss4->GetAttribute ("State", ptr);
    }
  Ptr <WifiPhyStateHelper> state = DynamicCast <WifiPhyStateHelper> (ptr.Get<WifiPhyStateHelper> ());
  currentState = state->GetState ();
  NS_TEST_ASSERT_MSG_EQ (currentState, expectedState, "PHY State " << currentState << " does not match expected state " << expectedState << " at " << Simulator::Now ());
}

void
TestStaticChannelBonding::CheckSecondaryChannelStatus (bool expectedIdle, uint8_t bss)
{
  bool currentlyIdle;
  PointerValue ptr;
  if (bss == 1)
    {
      m_rxPhyBss1->GetAttribute ("State", ptr);
    }
  else if (bss == 2)
    {
      m_rxPhyBss2->GetAttribute ("State", ptr);
    }
  else if (bss == 3)
    {
      m_rxPhyBss3->GetAttribute ("State", ptr);
    }
  else if (bss == 4)
    {
      m_rxPhyBss4->GetAttribute ("State", ptr);
    }
  Ptr <WifiPhyStateHelper> state = DynamicCast <WifiPhyStateHelper> (ptr.Get<WifiPhyStateHelper> ());
  currentlyIdle = state->IsSecondaryChannelIdle ();
  NS_TEST_ASSERT_MSG_EQ (currentlyIdle, expectedIdle, "Secondary channel status " << currentlyIdle << " does not match expected status " << expectedIdle << " at " << Simulator::Now ());
}

void
TestStaticChannelBonding::SendPacket (uint8_t bss)
{
  Ptr<BondingTestSpectrumWifiPhy> phy;
  uint16_t channelWidth = 20;
  uint32_t payloadSize = 1000;
  if (bss == 1)
    {
      phy = m_txPhyBss1;
      channelWidth = 20;
      payloadSize = 1001;
    }
  else if (bss == 2)
    {
      phy = m_txPhyBss2;
      channelWidth = 20;
      payloadSize = 1002;
    }
  else if (bss == 3)
    {
      phy = m_txPhyBss3;
      channelWidth = 40;
      payloadSize = 2100; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }
  else if (bss == 4)
    {
      phy = m_txPhyBss4;
      channelWidth = 40;
      payloadSize = 2101; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }

  WifiTxVector txVector = WifiTxVector (WifiPhy::GetHtMcs7 (), 0, WIFI_PREAMBLE_HT_MF, 800, 1, 1, 0, channelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (payloadSize);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  phy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestStaticChannelBonding::RxCallback (std::string context, Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW)
{
  uint32_t size = p->GetSize ();
  NS_LOG_INFO (context << " received packet with size " << size);
  if (context == "BSS1") //RX is in BSS 1
    {
      auto band = m_rxPhyBss1->GetBand(20, 0);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 1 received packet with size " << size << " and power in 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
    }
  else if (context == "BSS2") //RX is in BSS 2
    {
      auto band = m_rxPhyBss2->GetBand(20, 0);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 2 received packet with size " << size << " and power in 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
    }
  else if (context == "BSS3") //RX is in BSS 3
    {
      auto band = m_rxPhyBss3->GetBand(20, 0);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 3 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }

      band = m_rxPhyBss3->GetBand(20, 1);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 3 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }
    }
  else if (context == "BSS4") //RX is in BSS 4
    {
      auto band = m_rxPhyBss3->GetBand(20, 1);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 4 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 4 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }

      band = m_rxPhyBss3->GetBand(20, 0);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 4 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 4 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }
    }
}

void
TestStaticChannelBonding::RxOkCallback (std::string context, Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble)
{
  NS_LOG_INFO ("RxOkCallback: BSS=" << context << " SNR=" << RatioToDb (snr));
  if (context == "BSS1")
    {
      m_receptionBss1 = true;
      m_phyPayloadReceivedSuccessBss1 = true;
      if (m_initializedSnrBss1)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss1, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS2")
    {
      m_receptionBss2 = true;
      m_phyPayloadReceivedSuccessBss2 = true;
      if (m_initializedSnrBss2)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss2, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS3")
    {
      m_receptionBss3 = true;
      m_phyPayloadReceivedSuccessBss3 = true;
      if (m_initializedSnrBss3)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss3, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS4")
    {
      m_receptionBss4 = true;
      m_phyPayloadReceivedSuccessBss4 = true;
      if (m_initializedSnrBss4)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss4, 0.2, "Unexpected SNR value");
        }
    }
}

void
TestStaticChannelBonding::RxErrorCallback (std::string context, Ptr<const Packet> p, double snr)
{
  NS_LOG_INFO ("RxErrorCallback: BSS=" << context << " SNR=" << RatioToDb (snr));
  if (context == "BSS1")
    {
      m_receptionBss1 = true;
      m_phyPayloadReceivedSuccessBss1 = false;
      if (m_initializedSnrBss1)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss1, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS2")
    {
      m_receptionBss2 = true;
      m_phyPayloadReceivedSuccessBss2 = false;
      if (m_initializedSnrBss2)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss2, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS3")
    {
      m_receptionBss3 = true;
      m_phyPayloadReceivedSuccessBss3 = false;
      if (m_initializedSnrBss3)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss3, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS4")
    {
      m_receptionBss4 = true;
      m_phyPayloadReceivedSuccessBss4 = false;
      if (m_initializedSnrBss4)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss4, 0.2, "Unexpected SNR value");
        }
    }
}

TestStaticChannelBonding::~TestStaticChannelBonding ()
{
  m_rxPhyBss1 = 0;
  m_rxPhyBss2 = 0;
  m_rxPhyBss3 = 0;
  m_rxPhyBss4 = 0;
  m_txPhyBss1 = 0;
  m_txPhyBss2 = 0;
  m_txPhyBss3 = 0;
  m_txPhyBss4 = 0;
}

void
TestStaticChannelBonding::DoSetup (void)
{
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (50); // set default loss to 50 dB for all links
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  m_rxPhyBss1 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss1 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss1->SetPosition (Vector (1.0, 0.0, 0.0));
  m_rxPhyBss1->SetMobility (rxMobilityBss1);
  m_rxPhyBss1->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss1->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss1->SetChannel (channel);
  m_rxPhyBss1->SetErrorRateModel (error);
  m_rxPhyBss1->SetChannelNumber (36);
  m_rxPhyBss1->SetFrequency (5180);
  m_rxPhyBss1->SetChannelWidth (20);
  m_rxPhyBss1->SetTxPowerStart (0.0);
  m_rxPhyBss1->SetTxPowerEnd (0.0);
  m_rxPhyBss1->SetRxSensitivity (-91.0);
  m_rxPhyBss1->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss1->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss1->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss1->Initialize ();

  m_txPhyBss1 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss1 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss1->SetPosition (Vector (0.0, 0.0, 0.0));
  m_txPhyBss1->SetMobility (txMobilityBss1);
  m_txPhyBss1->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss1->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss1->SetChannel (channel);
  m_txPhyBss1->SetErrorRateModel (error);
  m_txPhyBss1->SetChannelNumber (36);
  m_txPhyBss1->SetFrequency (5180);
  m_txPhyBss1->SetChannelWidth (20);
  m_txPhyBss1->SetTxPowerStart (0.0);
  m_txPhyBss1->SetTxPowerEnd (0.0);
  m_txPhyBss1->SetRxSensitivity (-91.0);
  m_txPhyBss1->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss1->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss1->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss1->Initialize ();

  m_rxPhyBss2 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss2 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss2->SetPosition (Vector (1.0, 10.0, 0.0));
  m_rxPhyBss2->SetMobility (rxMobilityBss2);
  m_rxPhyBss2->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss2->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss2->SetChannel (channel);
  m_rxPhyBss2->SetErrorRateModel (error);
  m_rxPhyBss2->SetChannelNumber (40);
  m_rxPhyBss2->SetFrequency (5200);
  m_rxPhyBss2->SetChannelWidth (20);
  m_rxPhyBss2->SetTxPowerStart (0.0);
  m_rxPhyBss2->SetTxPowerEnd (0.0);
  m_rxPhyBss2->SetRxSensitivity (-91.0);
  m_rxPhyBss2->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss2->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss2->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss2->Initialize ();

  m_txPhyBss2 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss2 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss2->SetPosition (Vector (0.0, 10.0, 0.0));
  m_txPhyBss2->SetMobility (txMobilityBss2);
  m_txPhyBss2->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss2->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss2->SetChannel (channel);
  m_txPhyBss2->SetErrorRateModel (error);
  m_txPhyBss2->SetChannelNumber (40);
  m_txPhyBss2->SetFrequency (5200);
  m_txPhyBss2->SetChannelWidth (20);
  m_txPhyBss2->SetTxPowerStart (0.0);
  m_txPhyBss2->SetTxPowerEnd (0.0);
  m_txPhyBss2->SetRxSensitivity (-91.0);
  m_txPhyBss2->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss2->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss2->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss2->Initialize ();

  m_rxPhyBss3 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss3 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss3->SetPosition (Vector (1.0, 20.0, 0.0));
  m_rxPhyBss3->SetMobility (rxMobilityBss3);
  m_rxPhyBss3->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss3->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss3->SetChannel (channel);
  m_rxPhyBss3->SetErrorRateModel (error);
  m_rxPhyBss3->SetChannelNumber (38);
  m_rxPhyBss3->SetFrequency (5190);
  m_rxPhyBss3->SetChannelWidth (40);
  m_rxPhyBss3->SetSecondaryChannelOffset (UPPER);
  m_rxPhyBss3->SetTxPowerStart (0.0);
  m_rxPhyBss3->SetTxPowerEnd (0.0);
  m_rxPhyBss3->SetRxSensitivity (-91.0);
  m_rxPhyBss3->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss3->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss3->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss3->Initialize ();

  m_txPhyBss3 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss3 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss3->SetPosition (Vector (0.0, 20.0, 0.0));
  m_txPhyBss3->SetMobility (txMobilityBss3);
  m_txPhyBss3->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss3->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss3->SetChannel (channel);
  m_txPhyBss3->SetErrorRateModel (error);
  m_txPhyBss3->SetChannelNumber (38);
  m_txPhyBss3->SetFrequency (5190);
  m_txPhyBss3->SetChannelWidth (40);
  m_txPhyBss3->SetSecondaryChannelOffset (UPPER);
  m_txPhyBss3->SetTxPowerStart (0.0);
  m_txPhyBss3->SetTxPowerEnd (0.0);
  m_txPhyBss3->SetRxSensitivity (-91.0);
  m_txPhyBss3->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss3->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss3->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss3->Initialize ();

  m_rxPhyBss4 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss4 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss4->SetPosition (Vector (1.0, 30.0, 0.0));
  m_rxPhyBss4->SetMobility (rxMobilityBss4);
  m_rxPhyBss4->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss4->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss4->SetChannel (channel);
  m_rxPhyBss4->SetErrorRateModel (error);
  m_rxPhyBss4->SetChannelNumber (38);
  m_rxPhyBss4->SetFrequency (5190);
  m_rxPhyBss4->SetChannelWidth (40);
  m_rxPhyBss4->SetSecondaryChannelOffset (LOWER);
  m_rxPhyBss4->SetTxPowerStart (0.0);
  m_rxPhyBss4->SetTxPowerEnd (0.0);
  m_rxPhyBss4->SetRxSensitivity (-91.0);
  m_rxPhyBss4->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss4->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss4->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss4->Initialize ();

  m_txPhyBss4 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss4 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss4->SetPosition (Vector (0.0, 30.0, 0.0));
  m_txPhyBss4->SetMobility (txMobilityBss4);
  m_txPhyBss4->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss4->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss4->SetChannel (channel);
  m_txPhyBss4->SetErrorRateModel (error);
  m_txPhyBss4->SetChannelNumber (38);
  m_txPhyBss4->SetFrequency (5190);
  m_txPhyBss4->SetChannelWidth (40);
  m_txPhyBss4->SetSecondaryChannelOffset (LOWER);
  m_txPhyBss4->SetTxPowerStart (0.0);
  m_txPhyBss4->SetTxPowerEnd (0.0);
  m_txPhyBss4->SetRxSensitivity (-91.0);
  m_txPhyBss4->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss4->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss4->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss4->Initialize ();

  m_rxPhyBss1->TraceConnect ("PhyRxBegin", "BSS1", MakeCallback (&TestStaticChannelBonding::RxCallback, this));
  m_rxPhyBss2->TraceConnect ("PhyRxBegin", "BSS2", MakeCallback (&TestStaticChannelBonding::RxCallback, this));
  m_rxPhyBss3->TraceConnect ("PhyRxBegin", "BSS3", MakeCallback (&TestStaticChannelBonding::RxCallback, this));
  m_rxPhyBss4->TraceConnect ("PhyRxBegin", "BSS4", MakeCallback (&TestStaticChannelBonding::RxCallback, this));
  m_rxPhyBss1->GetState()->TraceConnect ("RxOk", "BSS1", MakeCallback (&TestStaticChannelBonding::RxOkCallback, this));
  m_rxPhyBss2->GetState()->TraceConnect ("RxOk", "BSS2", MakeCallback (&TestStaticChannelBonding::RxOkCallback, this));
  m_rxPhyBss3->GetState()->TraceConnect ("RxOk", "BSS3", MakeCallback (&TestStaticChannelBonding::RxOkCallback, this));
  m_rxPhyBss4->GetState()->TraceConnect ("RxOk", "BSS4", MakeCallback (&TestStaticChannelBonding::RxOkCallback, this));
  m_rxPhyBss1->GetState()->TraceConnect ("RxError", "BSS1", MakeCallback (&TestStaticChannelBonding::RxErrorCallback, this));
  m_rxPhyBss2->GetState()->TraceConnect ("RxError", "BSS2", MakeCallback (&TestStaticChannelBonding::RxErrorCallback, this));
  m_rxPhyBss3->GetState()->TraceConnect ("RxError", "BSS3", MakeCallback (&TestStaticChannelBonding::RxErrorCallback, this));
  m_rxPhyBss4->GetState()->TraceConnect ("RxError", "BSS4", MakeCallback (&TestStaticChannelBonding::RxErrorCallback, this));
}

void
TestStaticChannelBonding::DoRun (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  m_rxPhyBss1->AssignStreams (streamNumber);
  m_rxPhyBss2->AssignStreams (streamNumber);
  m_rxPhyBss3->AssignStreams (streamNumber);
  m_rxPhyBss4->AssignStreams (streamNumber);
  m_txPhyBss1->AssignStreams (streamNumber);
  m_txPhyBss2->AssignStreams (streamNumber);
  m_txPhyBss3->AssignStreams (streamNumber);
  m_txPhyBss4->AssignStreams (streamNumber);

  //CASE 1: each BSS send a packet on its channel to verify the received power per band for each receiver
  //and whether the packet is successfully received or not.*/

  //CASE 1A: BSS 1
  Simulator::Schedule (Seconds (0.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (1.0), &TestStaticChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, false, 3); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, false, 4); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, true, 3); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, true, 4); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (1.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 1); // successfull reception for BSS 1
  Simulator::Schedule (Seconds (1.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 3); // successfull reception for BSS 3
  Simulator::Schedule (Seconds (1.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 2); // no reception for BSS 2
  Simulator::Schedule (Seconds (1.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 4); // no reception for BSS 4

  //CASE 1B: BSS 2
  Simulator::Schedule (Seconds (1.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (2.0), &TestStaticChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, false, 4); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, false, 3); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, true, 3); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckSecondaryChannelStatus, this, true, 4); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (2.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 2); // successfull reception for BSS 2
  Simulator::Schedule (Seconds (2.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 4); // successfull reception for BSS 4
  Simulator::Schedule (Seconds (2.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 1); // no reception for BSS 1
  Simulator::Schedule (Seconds (2.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 3); // no reception for BSS 3

  //CASE 1C: BSS 3
  Simulator::Schedule (Seconds (2.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (3.0), &TestStaticChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (3.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 3); // successfull reception for BSS 3
  Simulator::Schedule (Seconds (3.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 4); // successfull reception for BSS 4
  Simulator::Schedule (Seconds (3.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 1); // no reception for BSS 1 since channel width is not supported
  Simulator::Schedule (Seconds (3.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 2); // no reception for BSS 2 since channel width is not supported

  //CASE 1D: BSS 4
  Simulator::Schedule (Seconds (3.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (4.0), &TestStaticChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (4.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 3); // successfull reception for BSS 3
  Simulator::Schedule (Seconds (4.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 4); // successfull reception for BSS 4
  Simulator::Schedule (Seconds (4.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 1); // no reception for BSS 1 since channel width is not supported
  Simulator::Schedule (Seconds (4.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 2); // no reception for BSS 2 since channel width is not supported

  //CASE 2: verify reception on channel 36 (BSS 1) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (4.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 44.0, 1); // BSS 1 expects SNR around 44 dB
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 44.0, 1); // BSS 2 expects SNR around 44 dB
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (5.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 1); // successfull reception for BSS 1
  Simulator::Schedule (Seconds (5.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, true, 2); // successfull reception for BSS 2

  //CASE 3: verify reception on channel 38 (BSS 3) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (5.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (6.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (6.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 3); // PHY header passed but payload failed for BSS 3

  //CASE 4: verify reception on channel 38 (BSS 3) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (6.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (7.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (7.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 3); // PHY header passed but payload failed for BSS 3

  //CASE 5: verify reception on channel 38 (BSS 4) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (7.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (8.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 1); // PHY header failed failed for BSS 1, so reception was aborted
  Simulator::Schedule (Seconds (8.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 4); // PHY header passed but payload failed for BSS 4

  //CASE 6: verify reception on channel 38 (BSS 4) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (8.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (9.5), &TestStaticChannelBonding::VerifyResultsForBss, this, false, false, 2); // PHY header failed failed for BSS 2, so reception was aborted
  Simulator::Schedule (Seconds (9.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 4); // PHY header passed but payload failed for BSS 4

  //CASE 7: verify reception on channel 38 (BSS 3) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (9.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (10.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (10.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (10.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 3); // PHY header passed but payload failed for BSS 3

  //CASE 8: verify reception on channel 38 (BSS 4) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (10.9), &TestStaticChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBonding::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (11.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (11.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (11.5), &TestStaticChannelBonding::VerifyResultsForBss, this, true, false, 4); // PHY header passed but payload failed for BSS 4

  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief dynamic channel bonding
 *
 * In this test, we have three 802.11n transmitters and three 802.11n receivers.
 * A BSS is composed of one transmitter and one receiver.
 *
 * The first BSS 1 makes uses of channel bonding on channel 38 (= 36 + 40),
 * with its secondary channel upper than its primary channel.
 * The second BSS operates on channel 40 with a channel width of 20 MHz.
 * Both BSS 3 is configured similarly as BSS 1 but has its secondary channel
 * lower than its primary channel.
 *
 */
class TestDynamicChannelBonding : public TestCase
{
public:
  TestDynamicChannelBonding ();
  virtual ~TestDynamicChannelBonding ();

protected:
  virtual void DoSetup (void);
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss1; ///< RX Phy BSS #1
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss2; ///< RX Phy BSS #2
  Ptr<BondingTestSpectrumWifiPhy> m_rxPhyBss3; ///< RX Phy BSS #3
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss1; ///< TX Phy BSS #1
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss2; ///< TX Phy BSS #2
  Ptr<BondingTestSpectrumWifiPhy> m_txPhyBss3; ///< TX Phy BSS #3

  /**
   * Send packet function
   * \param bss the BSS of the transmitter belongs to
   * \param expectedChannelWidth the expected channel width used for the transmission
   */
  void SendPacket (uint8_t bss, uint16_t expectedChannelWidth);

private:
  virtual void DoRun (void);
};

TestDynamicChannelBonding::TestDynamicChannelBonding ()
  : TestCase ("Dynamic channel bonding test")
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);
  //LogComponentEnable ("ConstantThresholdChannelBondingManager", logLevel);
  //LogComponentEnable ("WifiPhy", logLevel);
}

TestDynamicChannelBonding::~TestDynamicChannelBonding ()
{
  m_rxPhyBss1 = 0;
  m_rxPhyBss2 = 0;
  m_rxPhyBss3 = 0;
  m_txPhyBss1 = 0;
  m_txPhyBss2 = 0;
  m_txPhyBss3 = 0;
}

void
TestDynamicChannelBonding::SendPacket (uint8_t bss, uint16_t expectedChannelWidth)
{
  Ptr<BondingTestSpectrumWifiPhy> phy;
  uint32_t payloadSize = 1000;
  if (bss == 1)
    {
      phy = m_txPhyBss1;
      payloadSize = 1001;
    }
  else if (bss == 2)
    {
      phy = m_txPhyBss2;
      payloadSize = 1002;
    }
  else if (bss == 3)
    {
      phy = m_txPhyBss3;
      payloadSize = 1003;
    }
  uint16_t channelWidth = phy->GetUsableChannelWidth ();
  NS_TEST_ASSERT_MSG_EQ (channelWidth, expectedChannelWidth, "selected channel width is not as expected");

  WifiTxVector txVector = WifiTxVector (WifiPhy::GetHtMcs7 (), 0, WIFI_PREAMBLE_HT_MF, 800, 1, 1, 0, channelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (payloadSize);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  phy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestDynamicChannelBonding::DoSetup (void)
{
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (50); // set default loss to 50 dB for all links
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  m_rxPhyBss1 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss1 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss1->SetPosition (Vector (1.0, 20.0, 0.0));
  m_rxPhyBss1->SetMobility (rxMobilityBss1);
  m_rxPhyBss1->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss1->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss1->SetChannel (channel);
  m_rxPhyBss1->SetErrorRateModel (error);
  m_rxPhyBss1->SetChannelNumber (38);
  m_rxPhyBss1->SetFrequency (5190);
  m_rxPhyBss1->SetChannelWidth (40);
  m_rxPhyBss1->SetSecondaryChannelOffset (UPPER);
  m_rxPhyBss1->SetTxPowerStart (0.0);
  m_rxPhyBss1->SetTxPowerEnd (0.0);
  m_rxPhyBss1->SetRxSensitivity (-91.0);
  m_rxPhyBss1->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss1->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss1->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss1->Initialize ();

  m_txPhyBss1 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss1 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss1->SetPosition (Vector (0.0, 20.0, 0.0));
  m_txPhyBss1->SetMobility (txMobilityBss1);
  m_txPhyBss1->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss1->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss1->SetChannel (channel);
  m_txPhyBss1->SetErrorRateModel (error);
  m_txPhyBss1->SetChannelNumber (38);
  m_txPhyBss1->SetFrequency (5190);
  m_txPhyBss1->SetChannelWidth (40);
  m_txPhyBss1->SetSecondaryChannelOffset (UPPER);
  m_txPhyBss1->SetTxPowerStart (0.0);
  m_txPhyBss1->SetTxPowerEnd (0.0);
  m_txPhyBss1->SetRxSensitivity (-91.0);
  m_txPhyBss1->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss1->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss1->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss1->Initialize ();

  Ptr<ConstantThresholdChannelBondingManager> channelBondingManagerTx1 = CreateObject<ConstantThresholdChannelBondingManager> ();
  m_txPhyBss1->SetChannelBondingManager (channelBondingManagerTx1);
  m_txPhyBss1->SetPifs (MicroSeconds (25));

  m_rxPhyBss2 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss2 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss2->SetPosition (Vector (1.0, 10.0, 0.0));
  m_rxPhyBss2->SetMobility (rxMobilityBss2);
  m_rxPhyBss2->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss2->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss2->SetChannel (channel);
  m_rxPhyBss2->SetErrorRateModel (error);
  m_rxPhyBss2->SetChannelNumber (40);
  m_rxPhyBss2->SetFrequency (5200);
  m_rxPhyBss2->SetChannelWidth (20);
  m_rxPhyBss2->SetTxPowerStart (0.0);
  m_rxPhyBss2->SetTxPowerEnd (0.0);
  m_rxPhyBss2->SetRxSensitivity (-91.0);
  m_rxPhyBss2->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss2->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss2->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss2->Initialize ();

  m_txPhyBss2 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss2 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss2->SetPosition (Vector (0.0, 10.0, 0.0));
  m_txPhyBss2->SetMobility (txMobilityBss2);
  m_txPhyBss2->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss2->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss2->SetChannel (channel);
  m_txPhyBss2->SetErrorRateModel (error);
  m_txPhyBss2->SetChannelNumber (40);
  m_txPhyBss2->SetFrequency (5200);
  m_txPhyBss2->SetChannelWidth (20);
  m_txPhyBss2->SetTxPowerStart (0.0);
  m_txPhyBss2->SetTxPowerEnd (0.0);
  m_txPhyBss2->SetRxSensitivity (-91.0);
  m_txPhyBss2->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss2->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss2->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss2->Initialize ();

  Ptr<ConstantThresholdChannelBondingManager> channelBondingManagerTx2 = CreateObject<ConstantThresholdChannelBondingManager> ();
  m_txPhyBss2->SetChannelBondingManager (channelBondingManagerTx2);
  m_txPhyBss2->SetPifs (MicroSeconds (25));

  m_rxPhyBss3 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss3 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss3->SetPosition (Vector (1.0, 20.0, 0.0));
  m_rxPhyBss3->SetMobility (rxMobilityBss3);
  m_rxPhyBss3->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss3->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss3->SetChannel (channel);
  m_rxPhyBss3->SetErrorRateModel (error);
  m_rxPhyBss3->SetChannelNumber (38);
  m_rxPhyBss3->SetFrequency (5190);
  m_rxPhyBss3->SetChannelWidth (40);
  m_rxPhyBss3->SetSecondaryChannelOffset (LOWER);
  m_rxPhyBss3->SetTxPowerStart (0.0);
  m_rxPhyBss3->SetTxPowerEnd (0.0);
  m_rxPhyBss3->SetRxSensitivity (-91.0);
  m_rxPhyBss3->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_rxPhyBss3->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_rxPhyBss3->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_rxPhyBss3->Initialize ();

  m_txPhyBss3 = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss3 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss3->SetPosition (Vector (0.0, 20.0, 0.0));
  m_txPhyBss3->SetMobility (txMobilityBss3);
  m_txPhyBss3->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss3->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss3->SetChannel (channel);
  m_txPhyBss3->SetErrorRateModel (error);
  m_txPhyBss3->SetChannelNumber (38);
  m_txPhyBss3->SetFrequency (5190);
  m_txPhyBss3->SetChannelWidth (40);
  m_txPhyBss3->SetSecondaryChannelOffset (LOWER);
  m_txPhyBss3->SetTxPowerStart (0.0);
  m_txPhyBss3->SetTxPowerEnd (0.0);
  m_txPhyBss3->SetRxSensitivity (-91.0);
  m_txPhyBss3->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  m_txPhyBss3->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  m_txPhyBss3->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  m_txPhyBss3->Initialize ();

  Ptr<ConstantThresholdChannelBondingManager> channelBondingManagerTx3 = CreateObject<ConstantThresholdChannelBondingManager> ();
  m_txPhyBss3->SetChannelBondingManager (channelBondingManagerTx3);
  m_txPhyBss3->SetPifs (MicroSeconds (25));
}

void
TestDynamicChannelBonding::DoRun (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  m_rxPhyBss1->AssignStreams (streamNumber);
  m_rxPhyBss2->AssignStreams (streamNumber);
  m_txPhyBss1->AssignStreams (streamNumber);
  m_txPhyBss2->AssignStreams (streamNumber);

  //CASE 1: send on free channel, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (1.0), &TestDynamicChannelBonding::SendPacket, this, 1, 40);

  //CASE 2: send when secondardy channel is free for more than PIFS, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (2.0), &TestDynamicChannelBonding::SendPacket, this, 2, 20);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (164) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestDynamicChannelBonding::SendPacket, this, 1, 40);

  //CASE 3: send when secondary channel is free for less than PIFS, so BSS 1 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (3.0), &TestDynamicChannelBonding::SendPacket, this, 2, 20);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (164) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestDynamicChannelBonding::SendPacket, this, 1, 20);

  //Case 4: both transmitters send at the same time when channel was previously idle, BSS 1 shall anyway transmit at 40 MHz since it shall already indicate the selected channel width in its PHY header
  Simulator::Schedule (Seconds (4.0), &TestDynamicChannelBonding::SendPacket, this, 2, 20);
  Simulator::Schedule (Seconds (4.0), &TestDynamicChannelBonding::SendPacket, this, 1, 40);

  //Case 5: send when secondardy channel is free for more than PIFS, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (5.0), &TestDynamicChannelBonding::SendPacket, this, 3, 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestDynamicChannelBonding::SendPacket, this, 1, 40);

  //Case 6: send when secondardy channel is free for more than PIFS, so BSS 3 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (6.0), &TestDynamicChannelBonding::SendPacket, this, 1, 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestDynamicChannelBonding::SendPacket, this, 3, 40);

  //CASE 7: send when secondary channel is free for less than PIFS, so BSS 1 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (7.0), &TestDynamicChannelBonding::SendPacket, this, 3, 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestDynamicChannelBonding::SendPacket, this, 1, 20);

  //CASE 8: send when secondary channel is free for less than PIFS, so BSS 3 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (8.0), &TestDynamicChannelBonding::SendPacket, this, 1, 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestDynamicChannelBonding::SendPacket, this, 3, 20);

  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief effective SNR calculations
 */
class TestEffectiveSnrCalculations : public TestCase
{
public:
  TestEffectiveSnrCalculations ();
  virtual ~TestEffectiveSnrCalculations ();

private:
  virtual void DoSetup (void);
  virtual void DoRun (void);

  struct InterferenceInfo
  {
    uint16_t frequency; ///< Interference frequency in MHz
    uint16_t channelWidth; ///< Interference channel width in MHz
    double powerDbm; ///< Interference power in dBm
    InterferenceInfo(uint16_t freq, uint16_t width, double pow) : frequency (freq), channelWidth (width), powerDbm (pow) {}
  };

  /**
   * Run one function
   */
  void RunOne (void);

  /**
   * Generate interference function
   * \param phy the PHY to use to generate the interference
   * \param interference the structure holding the interference info
   */
  void GenerateInterference (Ptr<WaveformGenerator> phy, InterferenceInfo interference);
  /**
   * Stop interference function
   * \param phy the PHY to stop
   */
  void StopInterference (Ptr<WaveformGenerator> phy);

  /**
   * Send packet function
   */
  void SendPacket (void);

  /**
   * Callback triggered when a packet has been successfully received
   * \param p the received packet
   * \param snr the signal to noise ratio
   * \param mode the mode used for the transmission
   * \param preamble the preamble used for the transmission
   */
  void RxOkCallback (Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble);

  /**
   * Callback triggered when a packet has been unsuccessfully received
   * \param p the packet
   * \param snr the signal to noise ratio
   */
  void RxErrorCallback (Ptr<const Packet> p, double snr);

  Ptr<BondingTestSpectrumWifiPhy> m_rxPhy = 0; ///< Rx Phy
  Ptr<BondingTestSpectrumWifiPhy> m_txPhy = 0; ///< Tx Phy
  std::vector<Ptr<WaveformGenerator> > m_interferersPhys; ///< Interferers Phys
  uint16_t m_signalFrequency; ///< Signal frequency in MHz
  uint16_t m_signalChannelWidth; ///< Signal channel width in MHz
  double m_expectedSnrDb; ///< Expected SNR in dB
  uint32_t m_rxCount; ///< Counter for both RxOk and RxError callbacks

  std::vector<InterferenceInfo> m_interferences;
};

TestEffectiveSnrCalculations::TestEffectiveSnrCalculations ()
  : TestCase ("Effective SNR calculations test"),
    m_signalFrequency (5180),
    m_signalChannelWidth (20),
    m_rxCount (0)
{
}

TestEffectiveSnrCalculations::~TestEffectiveSnrCalculations ()
{
  m_rxPhy = 0;
  m_txPhy = 0;
  for (auto it = m_interferersPhys.begin (); it != m_interferersPhys.end (); it++)
    {
      *it = 0;
    }
  m_interferersPhys.clear ();
}

void
TestEffectiveSnrCalculations::GenerateInterference (Ptr<WaveformGenerator> phy, TestEffectiveSnrCalculations::InterferenceInfo interference)
{
  NS_LOG_INFO ("GenerateInterference: PHY=" << phy << " frequency=" << interference.frequency << " channelWidth=" << interference.channelWidth << " powerDbm=" << interference.powerDbm);
  BandInfo bandInfo;
  bandInfo.fc = interference.frequency * 1e6;
  bandInfo.fl = bandInfo.fc - (((interference.channelWidth / 2) + 1) * 1e6);
  bandInfo.fh = bandInfo.fc + (((interference.channelWidth / 2) - 1) * 1e6);
  Bands bands;
  bands.push_back (bandInfo);

  Ptr<SpectrumModel> spectrumInterference = Create<SpectrumModel> (bands);
  Ptr<SpectrumValue> interferencePsd = Create<SpectrumValue> (spectrumInterference);
  *interferencePsd = DbmToW (interference.powerDbm) / ((interference.channelWidth - 1) * 1e6);

  Time interferenceDuration = MilliSeconds (100);

  phy->SetTxPowerSpectralDensity (interferencePsd);
  phy->SetPeriod (interferenceDuration);
  phy->Start ();

  Simulator::Schedule (interferenceDuration, &TestEffectiveSnrCalculations::StopInterference, this, phy);
}

void
TestEffectiveSnrCalculations::StopInterference (Ptr<WaveformGenerator> phy)
{
  phy->Stop();
}

void
TestEffectiveSnrCalculations::SendPacket (void)
{
  WifiTxVector txVector = WifiTxVector (WifiPhy::GetVhtMcs7 (), 0, WIFI_PREAMBLE_VHT_SU, 800, 1, 1, 0, m_signalChannelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (1000);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  m_txPhy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestEffectiveSnrCalculations::RxOkCallback (Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble)
{
  NS_LOG_INFO ("RxOkCallback: SNR=" << RatioToDb (snr) << " dB expected_SNR=" << m_expectedSnrDb << " dB");
  m_rxCount++;
  NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrDb, 0.1, "SNR is different than expected");
}

void
TestEffectiveSnrCalculations::RxErrorCallback (Ptr<const Packet> p, double snr)
{
  NS_LOG_INFO ("RxErrorCallback: SNR=" << RatioToDb (snr) << " dB expected_SNR=" << m_expectedSnrDb << " dB");
  m_rxCount++;
  NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrDb, 0.1, "SNR is different than expected");
}

void
TestEffectiveSnrCalculations::DoSetup (void)
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);

  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (0); // set default loss to 0 dB for simplicity, so RX power = TX power
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  m_rxPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobility = CreateObject<ConstantPositionMobilityModel> ();
  rxMobility->SetPosition (Vector (1.0, 0.0, 0.0));
  m_rxPhy->SetMobility (rxMobility);
  m_rxPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  m_rxPhy->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhy->SetChannel (channel);
  m_rxPhy->SetErrorRateModel (error);
  m_rxPhy->SetChannelNumber (50); // to support up to 160 MHz signals
  m_rxPhy->Initialize ();

  m_txPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobility = CreateObject<ConstantPositionMobilityModel> ();
  txMobility->SetPosition (Vector (0.0, 0.0, 0.0));
  m_txPhy->SetMobility (txMobility);
  m_txPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  m_txPhy->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhy->SetChannel (channel);
  m_txPhy->SetErrorRateModel (error);
  m_txPhy->SetChannelNumber (50); // to support up to 160 MHz signals
  m_txPhy->Initialize ();

  for (unsigned int i = 0; i < (160 / 20); i++)
    {
      Ptr<Node> interfererNode = CreateObject<Node> ();
      Ptr<NonCommunicatingNetDevice> interfererDev = CreateObject<NonCommunicatingNetDevice> ();
      Ptr<WaveformGenerator> phy = CreateObject<WaveformGenerator> ();
      phy->SetDevice (interfererDev);
      phy->SetChannel (channel);
      phy->SetDutyCycle (1);
      interfererNode->AddDevice (interfererDev);
      m_interferersPhys.push_back (phy);
    }

  m_rxPhy->GetState()->TraceConnectWithoutContext ("RxOk", MakeCallback (&TestEffectiveSnrCalculations::RxOkCallback, this));
  m_rxPhy->GetState()->TraceConnectWithoutContext ("RxError", MakeCallback (&TestEffectiveSnrCalculations::RxErrorCallback, this));
}

void
TestEffectiveSnrCalculations::RunOne (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  m_rxPhy->AssignStreams (streamNumber);
  m_txPhy->AssignStreams (streamNumber);

  m_txPhy->SetTxPowerStart (18);
  m_txPhy->SetTxPowerEnd (18);

  Simulator::Schedule (Seconds (1.0), &TestEffectiveSnrCalculations::SendPacket, this);
  unsigned i = 0;
  for (auto const& interference : m_interferences)
    {
      Simulator::Schedule (Seconds (1.0) + MicroSeconds (40) + MicroSeconds (i), &TestEffectiveSnrCalculations::GenerateInterference, this, m_interferersPhys.at (i), interference);
      i++;
    }

  Simulator::Run ();

  m_interferences.clear ();
}

void
TestEffectiveSnrCalculations::DoRun (void)
{
  // Case 1: 20 MHz transmission: Reference case
  {
    m_signalFrequency = 5180;
    m_signalChannelWidth = 20;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 15));
    // SNR eff = SNR = 18 - 15 = 3 dB
    m_expectedSnrDb = 3;
    RunOne ();
  }

  // Case 2: 40 MHz transmission: I1 = I2
  {
    m_signalFrequency = 5190;
    m_signalChannelWidth = 40;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5190, 40, 15));
    // SNR eff,m = min ((18 - 3) - (15 - 3), (18 - 3) - (15 - 3)) = min (3 dB, 3 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(2)) = 12.5 = 10.9 dB
    m_expectedSnrDb = 10.9;
    RunOne ();
  }

  // Case 3: 40 MHz transmission: I2 = 0
  {
    m_signalFrequency = 5190;
    m_signalChannelWidth = 40;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 12));
    // SNR eff,m = min ((18 - 3) - 12, (18 - 3) - (-94)) min (3 dB, 109 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(2)) = 12.4 = 10.9 dB
    m_expectedSnrDb = 10.9;
    RunOne ();
  }

  // Case 4: 40 MHz transmission: I2 = 1/2 I1
  {
    m_signalFrequency = 5190;
    m_signalChannelWidth = 40;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 12));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 9));
    // SNR eff,m = min ((18 - 3) - 12, (18 - 3) - 9) = min (3 dB, 6 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(2)) = 12.4 = 10.9 dB
    m_expectedSnrDb = 10.9;
    RunOne ();
  }

  // Case 5: 80 MHz transmission: I1 = I2 = I3 = I4
  {
    m_signalFrequency = 5210;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5210, 80, 15));
    // SNR eff,m = min ((18 - 6) - (15 - 6), (18 - 6) - (15 - 6), (18 - 6) - (15 - 6), (18 - 6) - (15 - 6)) = min (3 dB, 3 dB, 3 dB, 3 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 6: 80 MHz transmission: I2 = I3 = I4 = 0
  {
    m_signalFrequency = 5210;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 9));
    // SNR eff,m = min ((18 - 6) - 9, (18 - 6) - (-94), (18 - 6) - (-94), (18 - 6) - (-94)) = min (3 dB, 106 dB, 106 dB, 106 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 7: 80 MHz transmission: I2 = 1/2 I1, I3 = I4 = 0
  {
    m_signalFrequency = 5210;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 9));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 6));
    // SNR eff,m = min ((18 - 6) - 9, (18 - 6) - 6, (18 - 6) - (-94), (18 - 6) - (-94)) = min (3 dB, 6 dB, 106 dB, 106 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 8: 80 MHz transmission: I2 = I3 = I4 = 1/2 I1
  {
    m_signalFrequency = 5210;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 9));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5220, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5240, 20, 6));
    // SNR eff,m = min ((18 - 6) - 9, (18 - 6) - 6, (18 - 6) - 6, (18 - 6) - 6) = min (3 dB, 6 dB, 6 dB, 6 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 9: 160 MHz transmission: I1 = I2 = I3 = I4 = I5 = I6 = I7 = I8
  {
    m_signalFrequency = 5250;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5250, 160, 15));
    // SNR eff,m = min ((18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9))
    //           = min (3 dB, 3 dB, 3 dB, 3 dB, 3 dB, 3 dB, 3 dB, 3 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  // Case 10: 160 MHz transmission: I2 = I3 = I4 = I5 = I6 = I7 = I8 = 0
  {
    m_signalFrequency = 5250;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 6));
    // SNR eff,m = min ((18 - 9) - 6, (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94))
    //           = min (3 dB, 103 dB, 103 dB, 103 dB, 103 dB, 103 dB, 103 dB, 103 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  // Case 11: 160 MHz transmission: I2 = I3 = I4 = 1/2 I1, I5 = I6 = I7 = I8 = 0
  {
    m_signalFrequency = 5250;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5220, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5240, 20, 3));
    // SNR eff,m = min ((18 - 9) - 6, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94))
    //           = min (3 dB, 6 dB, 6 dB, 6 dB, 103 dB, 103 dB, 103 dB, 103 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  // Case 12: 160 MHz transmission: I2 = I3 = I4 = I5 = I6 = I7 = I8 = 1/2 I1
  {
    m_signalFrequency = 5250;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5220, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5240, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5260, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5280, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5300, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5320, 20, 3));
    // SNR eff,m = min ((18 - 9) - 6, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3)
    //           = min (3 dB, 6 dB, 6 dB, 6 dB, 6 dB, 6 dB, 6 dB, 6 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  NS_TEST_EXPECT_MSG_EQ (m_rxCount, 12, "12 packets should have been received!");

  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief wifi channel bonding test suite
 */
class WifiChannelBondingTestSuite : public TestSuite
{
public:
  WifiChannelBondingTestSuite ();
};

WifiChannelBondingTestSuite::WifiChannelBondingTestSuite ()
  : TestSuite ("wifi-channel-bonding", UNIT)
{
  AddTestCase (new TestStaticChannelBonding, TestCase::QUICK);
  AddTestCase (new TestDynamicChannelBonding, TestCase::QUICK);
  AddTestCase (new TestEffectiveSnrCalculations, TestCase::QUICK);
}

static WifiChannelBondingTestSuite wifiChannelBondingTestSuite; ///< the test suite
