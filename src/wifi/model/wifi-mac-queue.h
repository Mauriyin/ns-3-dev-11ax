/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 *          Stefano Avallone <stavallo@unina.it>
 */

#ifndef WIFI_MAC_QUEUE_H
#define WIFI_MAC_QUEUE_H

#include "wifi-mac-queue-item.h"

namespace ns3 {

class QosBlockedDestinations;

/**
 * \ingroup wifi
 *
 * This queue implements the timeout procedure described in
 * (Section 9.19.2.6 "Retransmit procedures" paragraph 6; IEEE 802.11-2012).
 *
 * When a packet is received by the MAC, to be sent to the PHY,
 * it is queued in the internal queue after being tagged by the
 * current time.
 *
 * When a packet is dequeued, the queue checks its timestamp
 * to verify whether or not it should be dropped. If
 * dot11EDCATableMSDULifetime has elapsed, it is dropped.
 * Otherwise, it is returned to the caller.
 */
class WifiMacQueue : public Queue<WifiMacQueueItem>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  WifiMacQueue ();
  ~WifiMacQueue ();

  /// drop policy
  enum DropPolicy
  {
    DROP_NEWEST,
    DROP_OLDEST
  };

  /**
   * \brief Set the maximum size of this queue
   *
   * Trying to set a null size has no effect.
   *
   * \param size the maximum size
   */
  void SetMaxQueueSize (QueueSize size);
  /**
   * \return the maximum size of this queue
   */
  QueueSize GetMaxQueueSize (void) const;
  /**
   * Set the maximum delay before the packet is discarded.
   *
   * \param delay the maximum delay
   */
  void SetMaxDelay (Time delay);
  /**
   * Return the maximum delay before the packet is discarded.
   *
   * \return the maximum delay
   */
  Time GetMaxDelay (void) const;

  /**
   * Enqueue the given Wifi MAC queue item at the <i>end</i> of the queue.
   *
   * \param item the Wifi MAC queue item to be enqueued at the end
   * \return true if success, false if the packet has been dropped
   */
  bool Enqueue (Ptr<WifiMacQueueItem> item);
  /**
   * Enqueue the given Wifi MAC queue item at the <i>front</i> of the queue.
   *
   * \param item the Wifi MAC queue item to be enqueued at the front
   * \return true if success, false if the packet has been dropped
   */
  bool PushFront (Ptr<WifiMacQueueItem> item);
  /**
   * Dequeue the packet in the front of the queue.
   *
   * \return the packet
   */
  Ptr<WifiMacQueueItem> Dequeue (void);
  /**
   * Search and return, if present in the queue, the first packet having the
   * address indicated by <i>type</i> equal to <i>addr</i>.
   * This method removes the packet from the queue.
   * It is typically used by ns3::Txop during the CF period.
   *
   * \param dest the given destination
   *
   * \return the packet
   */
  Ptr<WifiMacQueueItem> DequeueByAddress (Mac48Address dest);
  /**
   * Search and return, if present in the queue, the first packet having the
   * address indicated by <i>type</i> equal to <i>addr</i>, and tid
   * equal to <i>tid</i>. This method removes the packet from the queue.
   * It is typically used by ns3::QosTxop in order to perform correct MSDU
   * aggregation (A-MSDU).
   *
   * \param tid the given TID
   * \param dest the given destination
   *
   * \return the packet
   */
  Ptr<WifiMacQueueItem> DequeueByTidAndAddress (uint8_t tid,
                                                Mac48Address dest);
  /**
   * Return first available packet for transmission. A packet could be no available
   * if it is a QoS packet with a tid and an address1 fields equal to <i>tid</i> and <i>addr</i>
   * respectively that index a pending agreement in the BlockAckManager object.
   * So that packet must not be transmitted until reception of an ADDBA response frame from station
   * addressed by <i>addr</i>. This method removes the packet from queue.
   *
   * \param blockedPackets
   *
   * \return packet
   */
  Ptr<WifiMacQueueItem> DequeueFirstAvailable (const Ptr<QosBlockedDestinations> blockedPackets);
  /**
   * Peek the packet in the front of the queue. The packet is not removed.
   *
   * \return the packet
   */
  Ptr<const WifiMacQueueItem> Peek (void) const;
  /**
   * Search and return, if present in the queue, the first packet having the
   * address indicated by <i>type</i> equal to <i>addr</i>, and tid
   * equal to <i>tid</i>. This method does not remove the packet from the queue.
   * It is typically used by ns3::QosTxop in order to perform correct MSDU
   * aggregation (A-MSDU).
   *
   * \param tid the given TID
   * \param dest the given destination
   *
   * \return packet
   */
  Ptr<const WifiMacQueueItem> PeekByTidAndAddress (uint8_t tid,
                                                   Mac48Address dest);
  /**
   * Return first available packet for transmission. The packet is not removed from queue.
   *
   * \param blockedPackets
   *
   * \return packet
   */
  Ptr<const WifiMacQueueItem> PeekFirstAvailable (const Ptr<QosBlockedDestinations> blockedPackets);
  /**
   * Remove the packet in the front of the queue.
   *
   * \return the packet
   */
  Ptr<WifiMacQueueItem> Remove (void);
  /**
   * If exists, removes <i>packet</i> from queue and returns true. Otherwise it
   * takes no effects and return false. Deletion of the packet is
   * performed in linear time (O(n)).
   *
   * \param packet the packet to be removed
   *
   * \return true if the packet was removed, false otherwise
   */
  bool Remove (Ptr<const Packet> packet);
  /**
   * Return the number of packets having destination address specified by
   * <i>dest</i>.
   *
   * \param dest the given destination
   *
   * \return the number of packets
   */
  uint32_t GetNPacketsByAddress (Mac48Address dest);
  /**
   * Return the number of QoS packets having tid equal to <i>tid</i> and
   * destination address equal to <i>dest</i>.
   *
   * \param tid the given TID
   * \param dest the given destination
   *
   * \return the number of QoS packets
   */
  uint32_t GetNPacketsByTidAndAddress (uint8_t tid, Mac48Address dest);

  /**
   * \return true if the queue is empty; false otherwise
   *
   * Overrides the IsEmpty method provided by QueueBase
   */
  bool IsEmpty (void);

  /**
   * \return The number of packets currently stored in the Queue
   *
   * Overrides the GetNPackets method provided by QueueBase
   */
  uint32_t GetNPackets (void);

  /**
   * \return The number of bytes currently occupied by the packets in the Queue
   *
   * Overrides the GetNBytes method provided by QueueBase
   */
  uint32_t GetNBytes (void);

private:
  /**
   * Remove the item pointed to by the iterator <i>it</i> if it has been in the
   * queue for too long. If the item is removed, the iterator is updated to
   * point to the item that followed the erased one.
   *
   * \param it an iterator pointing to the item
   * \return true if the item is removed, false otherwise
   */
  bool TtlExceeded (ConstIterator &it);

  QueueSize m_maxSize;                      //!< max queue size
  Time m_maxDelay;                          //!< Time to live for packets in the queue
  DropPolicy m_dropPolicy;                  //!< Drop behavior of queue

  NS_LOG_TEMPLATE_DECLARE;                  //!< redefinition of the log component
};

} //namespace ns3

#endif /* WIFI_MAC_QUEUE_H */
