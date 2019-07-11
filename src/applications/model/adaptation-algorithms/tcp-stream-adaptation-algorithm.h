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

#ifndef ADAPTATION_ALGORITHM_H
#define ADAPTATION_ALGORITHM_H
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <vector>
#include "ns3/application.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "tcp-stream-interface.h"

namespace ns3 {

class AdaptationAlgorithm : public Object {
 public:
  AdaptationAlgorithm(const videoData &videoData,
                      const playbackData &playbackData,
                      const bufferData &bufferData,
                      const throughputData &throughput);

  virtual algorithmReply GetNextRep(
      const int64_t segmentCounter, const int64_t clientId,
      int64_t extraParameter,   // usually this is bandwidth
      int64_t extraParameter2)  // usually this is not of use
      = 0;

 protected:
  const videoData &m_videoData;
  const bufferData &m_bufferData;
  const throughputData &m_throughput;
  const playbackData &m_playbackData;
};

}  // namespace ns3
#endif /* ADAPTATION_ALGORITHM_H */