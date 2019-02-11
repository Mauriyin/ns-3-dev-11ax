/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Sébastien Deronne
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

#ifndef WIFI_PHY_HEADER_H
#define WIFI_PHY_HEADER_H

#include "ns3/header.h"

namespace ns3 {

/**
 * \ingroup wifi
 *
 * Implements the IEEE 802.11 DSSS SIG PHY header
 */
class DsssSigHeader : public Header
{
public:
  DsssSigHeader ();
  virtual ~DsssSigHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Fill the RATE field of L-SIG (in bit/s).
   *
   * \param rate the RATE field of L-SIG expressed in bit/s
   */
  void SetRate (uint64_t rate);
  /**
   * Return the RATE field of L-SIG (in bit/s).
   *
   * \return the RATE field of L-SIG expressed in bit/s
   */
  uint64_t GetRate (void) const;
  /**
   * Fill the LENGTH field of L-SIG (in bytes).
   *
   * \param length the LENGTH field of L-SIG expressed in bytes
   */
  void SetLength (uint16_t length);
  /**
   * Return the LENGTH field of L-SIG (in bytes).
   *
   * \return the LENGTH field of L-SIG expressed in bytes
   */
  uint16_t GetLength (void) const;


private:
  uint8_t m_rate;    ///< RATE field
  uint16_t m_length; ///< LENGTH field
};


/**
 * \ingroup wifi
 *
 * Implements the IEEE 802.11 OFDM L-SIG PHY header
 */
class LSigHeader : public Header
{
public:
  LSigHeader ();
  virtual ~LSigHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Fill the RATE field of L-SIG (in bit/s).
   *
   * \param rate the RATE field of L-SIG expressed in bit/s
   */
  void SetRate (uint64_t rate);
  /**
   * Return the RATE field of L-SIG (in bit/s).
   *
   * \return the RATE field of L-SIG expressed in bit/s
   */
  uint64_t GetRate (void) const;
  /**
   * Fill the LENGTH field of L-SIG (in bytes).
   *
   * \param length the LENGTH field of L-SIG expressed in bytes
   */
  void SetLength (uint16_t length);
  /**
   * Return the LENGTH field of L-SIG (in bytes).
   *
   * \return the LENGTH field of L-SIG expressed in bytes
   */
  uint16_t GetLength (void) const;


private:
  uint8_t m_rate;    ///< RATE field
  uint16_t m_length; ///< LENGTH field
};


/**
 * \ingroup wifi
 *
 * Implements the IEEE 802.11n HT-SIG PHY header
 */
class HtSigHeader : public Header
{
public:
  HtSigHeader ();
  virtual ~HtSigHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Fill the MCS field of HT-SIG.
   *
   * \param length the MCS field of HT-SIG
   */
  void SetMcs (uint8_t mcs);
  /**
   * Return the MCS field of HT-SIG.
   *
   * \return the MCS field of HT-SIG
   */
  uint8_t GetMcs (void) const;
  /**
   * Fill the channel width field of HT-SIG (in MHz).
   *
   * \param width the channel width (in MHz)
   */
  void SetChannelWidth (uint16_t width);
  /**
   * Return the channel width (in MHz).
   *
   * \return the channel width (in MHz)
   */
  uint16_t GetChannelWidth (void) const;
  /**
   * Fill the aggregation field of HT-SIG .
   *
   * \param aggregation whether the PSDU contains A-MPDU or not
   */
  void SetAggregation (bool aggregation);
  /**
   * Return the aggregation field of HT-SIG.
   *
   * \return the aggregation field of HT-SIG
   */
  bool GetAggregation (void) const;
  /**
   * Fill the short guard interval field of HT-SIG.
   *
   * \param sgi whether short guard interval is used or not
   */
  void SetShortGuardInterval (bool sgi);
  /**
   * Return the short guard interval field of HT-SIG.
   *
   * \return the short guard interval field of HT-SIG
   */
  bool GetShortGuardInterval (void) const;
  /**
   * Fill the HT length field of HT-SIG (in bytes).
   *
   * \param length the HT length field of HT-SIG (in bytes)
   */
  void SetLength (uint16_t length);
  /**
   * Return the HT length field of HT-SIG (in bytes).
   *
   * \return the HT length field of HT-SIG (in bytes)
   */
  uint16_t GetLength (void) const;


private:
  uint8_t m_mcs;         ///< Modulation and Coding Scheme index
  uint8_t m_cbw20_40;    ///< CBW 20/40
  uint16_t m_length;     ///< HT length
  uint8_t m_aggregation; ///< Aggregation
  uint8_t m_sgi;         ///< Short Guard Interval
};

/**
 * \ingroup wifi
 *
 * Implements the IEEE 802.11ac PHY header (VHT-SIG-A1/A2/B)
 */
class VhtSigHeader : public Header
{
public:
  VhtSigHeader ();
  virtual ~VhtSigHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Fill the channel width field of VHT-SIG-A1 (in MHz).
   *
   * \param width the channel width (in MHz)
   */
  void SetChannelWidth (uint16_t width);
  /**
   * Return the channel width (in MHz).
   *
   * \return the channel width (in MHz)
   */
  uint16_t GetChannelWidth (void) const;
  /**
   * Fill the number of streams field of VHT-SIG-A1.
   *
   * \param nStreams the number of streams
   */
  void SetNStreams (uint8_t nStreams);
  /**
   * Return the number of streams.
   *
   * \return the number of streams
   */
  uint8_t GetNStreams (void) const;

  /**
   * Fill the short guard interval field of VHT-SIG-A2.
   *
   * \param sgi whether short guard interval is used or not
   */
  void SetShortGuardInterval (bool sgi);
  /**
   * Return the short GI field of VHT-SIG-A2.
   *
   * \return the short GI field of VHT-SIG-A2
   */
  bool GetShortGuardInterval (void) const;
  /**
   * Fill the short GI NSYM disambiguation field of VHT-SIG-A2.
   *
   * \param disambiguation whether short GI NSYM disambiguation is set or not
   */
  void SetShortGuardIntervalDisambiguation (bool disambiguation);
  /**
   * Return the short GI NSYM disambiguation field of VHT-SIG-A2.
   *
   * \return the short GI NSYM disambiguation field of VHT-SIG-A2
   */
  bool GetShortGuardIntervalDisambiguation (void) const;
  /**
   * Fill the SU VHT MCS field of VHT-SIG-A2.
   *
   * \param length the SU VHT MCS field of VHT-SIG-A2
   */
  void SetSuMcs (uint8_t mcs);
  /**
   * Return the SU VHT MCS field of VHT-SIG-A2.
   *
   * \return the SU VHT MCS field of VHT-SIG-A2
   */
  uint8_t GetSuMcs (void) const;


private:
  //VHT-SIG-A1 fields
  uint8_t m_bw;   ///< BW
  uint8_t m_nsts; ///< NSTS

  //VHT-SIG-A2 fields
  uint8_t m_sgi;                ///< Short GI
  uint8_t m_sgi_disambiguation; ///< Short GI NSYM Disambiguation
  uint8_t m_suMcs;              ///< SU VHT MCS

  //TODO: add boolean to know whether this is part of a VHT SU or VHT MU PPDU (fields and positions are different! We currently only support VHT SU)
};


/**
 * \ingroup wifi
 *
 * Implements the IEEE 802.11ax HE-SIG PHY header (HE-SIG-A1/A2/B)
 */
class HeSigHeader : public Header
{
public:
  HeSigHeader ();
  virtual ~HeSigHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Fill the MCS field of HE-SIG-A1.
   *
   * \param length the MCS field of HE-SIG-A1
   */
  void SetMcs (uint8_t mcs);
  /**
   * Return the MCS field of HE-SIG-A1.
   *
   * \return the MCS field of HE-SIG-A1
   */
  uint8_t GetMcs (void) const;
  /**
   * Fill the BSS Color field of HE-SIG-A1.
   *
   * \param bssColor the BSS Color value
   */
  void SetBssColor (uint8_t bssColor);
  /**
   * Return the BSS Color field in the HE-SIG-A1.
   *
   * \return the BSS Color field in the HE-SIG-A1
   */
  uint8_t GetBssColor (void) const;
  /**
   * Fill the channel width field of HE-SIG-A1 (in MHz).
   *
   * \param width the channel width (in MHz)
   */
  void SetChannelWidth (uint16_t width);
  /**
   * Return the channel width (in MHz).
   *
   * \return the channel width (in MHz)
   */
  uint16_t GetChannelWidth (void) const;
  /**
   * Fill the GI + LTF size field of HE-SIG-A1.
   *
   * \param gi the guard interval (in nanoseconds)
   * \param ltf the sequence of HE-LTF
   */
  void SetGuardIntervalAndLtfSize (uint16_t gi, uint8_t ltf);
  /**
   * Return the guard interval (in nanoseconds).
   *
   * \return the guard interval (in nanoseconds)
   */
  uint16_t GetGuardInterval (void) const;
  /**
   * Fill the number of streams field of HE-SIG-A1.
   *
   * \param nStreams the number of streams
   */
  void SetNStreams (uint8_t nStreams);
  /**
   * Return the number of streams.
   *
   * \return the number of streams
   */
  uint8_t GetNStreams (void) const;


private:
  //HE-SIG-A1 fields
  uint8_t m_format;       ///< Format bit
  uint8_t m_bssColor;     ///< BSS color field
  uint8_t m_ul_dl;        ///< UL/DL bit
  uint8_t m_mcs;          ///< MCS field
  uint8_t m_spatialReuse; ///< Spatial Reuse field
  uint8_t m_bandwidth;    ///< Bandwidth field
  uint8_t m_gi_ltf_size;  ///< GI+LTF Size field
  uint8_t m_nsts;         ///< NSTS
  //TODO: add boolean to know whether this is part of a HE SU or HE MU PPDU (fields and positions are different! We currently only support HE SU)
};

} //namespace ns3

#endif /* WIFI_PHY_HEADER_H */
