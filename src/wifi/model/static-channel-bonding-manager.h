/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef STATIC_CHANNEL_BONDING_MANAGER_H
#define STATIC_CHANNEL_BONDING_MANAGER_H

#include "channel-bonding-manager.h"

namespace ns3 {

/**
 * \brief Static Channel Bonding Manager
 * \ingroup wifi
 *
 * This object provides an implementation for determining whether the  bonded channels can be used or not.
 *
 */
class StaticChannelBondingManager : public ChannelBondingManager
{
public:
  StaticChannelBondingManager ();

  static TypeId GetTypeId (void);

  /**
   * Returns the selected channel width (in MHz).
   *
   * \return the  supported PHY channel width in MHz if transmission on bonded channels is allowed
   *         0 otherwise.
   */
  uint16_t GetUsableChannelWidth (void);
};

} //namespace ns3

#endif /* STATIC_CHANNEL_BONDING_MANAGER_H */
