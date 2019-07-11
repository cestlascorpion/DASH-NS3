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

#include "adapation-tobasco.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TobascoAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(TobascoAlgorithm);

TobascoAlgorithm::TobascoAlgorithm(const videoData &videoData,
                                   const playbackData &playbackData,
                                   const bufferData &bufferData,
                                   const throughputData &throughput)
    : AdaptationAlgorithm(videoData, playbackData, bufferData, throughput),
      m_a1(0.85),                                            // these param
      m_a2(0.33),                                            // are set
      m_a3(0.50),                                            // as the paper
      m_a4(0.75),                                            // say
      m_a5(0.95),                                            // do not chage it
      m_bMin(m_videoData.segmentDuration * 4),               // 2s
      m_bLow(m_videoData.segmentDuration * 8),               // 8s
      m_bHigh(m_videoData.segmentDuration * 12),             // 12s
      m_bOpt(m_videoData.segmentDuration * 10),              // 10s
      m_bufferUpperbound(m_videoData.segmentDuration * 15),  // upper bound 15s
      m_lastRepIndex(0),
      m_lastBuffer(0),
      m_runningFastStart(true),
      m_highestRepIndex(videoData.averageBitrate[0].size() - 1) {
  NS_LOG_INFO(this);
  NS_ASSERT_MSG(m_highestRepIndex >= 0,
                "The highest quality representation index should be >= 0");
}

algorithmReply TobascoAlgorithm::GetNextRep(const int64_t segmentCounter,
                                            const int64_t clientId,
                                            int64_t bandwidth) {
  int64_t decisionCase = 0;
  int64_t delayDecision = 0;
  int64_t nextRepIndex = 0;
  int64_t expectBuffer = 0;
  const int64_t timeNow = Simulator::Now().GetMicroSeconds();
  int64_t bufferNow = 0;
  if (segmentCounter != 0) {
    nextRepIndex = m_lastRepIndex;
    bufferNow = m_bufferData.bufferLevelNew.back() -
                (timeNow - m_throughput.transmissionEnd.back());

    int64_t nextHighestRepBitrate;
    if (m_lastRepIndex != m_highestRepIndex)
      nextHighestRepBitrate =
          m_videoData.averageBitrate.at(m_videoData.userInfo.at(segmentCounter))
              .at(m_lastRepIndex + 1);
    else
      nextHighestRepBitrate =
          m_videoData.averageBitrate.at(m_videoData.userInfo.at(segmentCounter))
              .at(m_lastRepIndex);

    bool isValid = segmentCounter >= 3
                       ? (m_videoData.averageBitrate
                              .at(m_videoData.userInfo.at(segmentCounter))
                              .at(m_lastRepIndex) <= m_a1 * bandwidth)
                       : true;
    if (m_runningFastStart && m_lastRepIndex != m_highestRepIndex &&
        bufferNow >= m_lastBuffer && isValid) {
      if (bufferNow < m_bMin) {
        if (nextHighestRepBitrate <= (m_a2 * bandwidth)) {
          decisionCase = 1;
          nextRepIndex = m_lastRepIndex + 1;
        }
      } else if (bufferNow < m_bLow) {
        if (nextHighestRepBitrate <= (m_a3 * bandwidth)) {
          decisionCase = 2;
          nextRepIndex = m_lastRepIndex + 1;
        }
      } else {
        if (nextHighestRepBitrate <= (m_a4 * bandwidth)) {
          decisionCase = 3;
          nextRepIndex = m_lastRepIndex + 1;
        }
        if (bufferNow > m_bHigh) {
          decisionCase = 4;
          delayDecision = 1;
          expectBuffer = m_bHigh - m_videoData.segmentDuration;
        }
      }
    } else {
      m_runningFastStart = false;
      if (bufferNow < m_bMin) {
        decisionCase = 5;
        nextRepIndex = 0;
      } else if (bufferNow < m_bLow) {
        int64_t lastSegmentThroughput =
            8 *
            (m_videoData.segmentSize
                 .at(m_videoData.userInfo.at(segmentCounter - 1))
                 .at(m_lastRepIndex)
                 .at(segmentCounter - 1)) *
            1000000 /
            (m_throughput.transmissionEnd.at(segmentCounter - 1) -
             m_throughput.transmissionStart.at(segmentCounter - 1));
        if ((m_lastRepIndex != 0) &&
            (m_videoData.averageBitrate
                 .at(m_videoData.userInfo.at(segmentCounter))
                 .at(m_lastRepIndex) >= lastSegmentThroughput)) {
          decisionCase = 6;
          nextRepIndex = m_lastRepIndex - 1;
        }
      } else if (bufferNow < m_bHigh) {
        if ((m_lastRepIndex == m_highestRepIndex) ||
            (nextHighestRepBitrate >= m_a5 * bandwidth)) {
          decisionCase = 7;
          delayDecision = 2;
          expectBuffer = (int64_t)(
              std::max(bufferNow - m_videoData.segmentDuration, m_bOpt));
        }
      } else {
        if ((m_lastRepIndex == m_highestRepIndex) ||
            (nextHighestRepBitrate >= m_a5 * bandwidth)) {
          decisionCase = 8;
          delayDecision = 3;
          expectBuffer = (int64_t)(
              std::max(bufferNow - m_videoData.segmentDuration, m_bOpt));
        } else {
          decisionCase = 9;
          nextRepIndex = m_lastRepIndex + 1;
        }
      }
    }
  }

  algorithmReply answer;
  answer.nextRepIndex = nextRepIndex;
  answer.decisionTime = timeNow;
  answer.decisionCase = decisionCase;
  answer.nextDownloadDelay = 0;
  answer.delayDecisionCase = delayDecision;
  answer.estimateTh = bandwidth;

  if (segmentCounter != 0 && delayDecision != 0) {
    if (expectBuffer > bufferNow) {
      answer.nextDownloadDelay = 0;
    } else {
      answer.nextDownloadDelay = bufferNow - expectBuffer;
    }
  }

  // add for xhinaxobile
  expectBuffer = std::max(expectBuffer, bufferNow);
  if (expectBuffer > m_bufferUpperbound - m_videoData.segmentDuration) {
    answer.nextDownloadDelay +=
        expectBuffer - (m_bufferUpperbound - m_videoData.segmentDuration);
  }
  answer.nextDownloadDelay =
      answer.nextDownloadDelay >= 0 ? answer.nextDownloadDelay : 0;
  //

  m_lastRepIndex = nextRepIndex;
  m_lastBuffer = bufferNow;
  return answer;
}

}  // namespace ns3