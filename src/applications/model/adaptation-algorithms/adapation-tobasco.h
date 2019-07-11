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

#ifndef TOBASCO_ALGORITHM_H
#define TOBASCO_ALGORITHM_H
#include "tcp-stream-adaptation.h"

namespace ns3 {

/**
 * \ingroup tcpStream
 * \brief Implementation of the Tobasco adaptation algorithm
 */
class TobascoAlgorithm : public AdaptationAlgorithm {
 public:
  TobascoAlgorithm(const videoData &videoData, const playbackData &playbackData,
                   const bufferData &bufferData,
                   const throughputData &throughput);

  algorithmReply GetNextRep(const int64_t segmentCounter,
                            const int64_t clientId, int64_t bandwidth);

 private:
  const double m_a1;
  const double m_a2;
  const double m_a3;
  const double m_a4;
  const double m_a5;
  const int64_t m_bMin;
  const int64_t m_bLow;
  const int64_t m_bHigh;
  const int64_t m_bOpt;
  const int64_t m_bufferUpperbound;
  int64_t m_lastRepIndex;
  int64_t m_lastBuffer;
  bool m_runningFastStart;
  const int64_t m_highestRepIndex;
};
}  // namespace ns3
#endif /* TOBASCO_ALGORITHM_H */