#include "adapation-sara.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SaraAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(SaraAlgorithm);

SaraAlgorithm::SaraAlgorithm(const videoData &videoData,
                             const playbackData &playbackData,
                             const bufferData &bufferData,
                             const throughputData &throughput)
    : AdaptationAlgorithm(videoData, playbackData, bufferData, throughput),
      m_lastRepIndex(0),                               // last bitrate level
      m_bufferHigh(m_videoData.segmentDuration * 10),  // 10s
      m_bufferLow(m_videoData.segmentDuration * 8),    // 8s
      m_bufferMin(m_videoData.segmentDuration * 6),    // 6s
      m_bufferUpperbound(m_videoData.segmentDuration * 15),  // 15s
      m_highestRepIndex(videoData.averageBitrate[0].size() - 1) {
  NS_LOG_INFO(this);
  NS_ASSERT_MSG(m_highestRepIndex >= 0,
                "The highest quality representation index should be >= 0");
}

algorithmReply SaraAlgorithm::GetNextRep(const int64_t segmentCounter,
                                         const int64_t clientId,
                                         int64_t bandwidth) {
  algorithmReply answer;
  answer.decisionCase = 0;
  answer.delayDecisionCase = 0;
  answer.nextDownloadDelay = 0;
  const int64_t timeNow = Simulator::Now().GetMicroSeconds();
  answer.decisionTime = timeNow;
  int64_t bufferNow = 0;
  if (segmentCounter != 0) {
    bufferNow = m_bufferData.bufferLevelNew.back() -
                (timeNow - m_throughput.transmissionEnd.back());
    if (bufferNow <= m_bufferMin) {
      answer.nextRepIndex = 0;
      answer.decisionCase = 1;
    } else {
      if ((double)(8.0 * m_videoData.segmentSize
                             .at(m_videoData.userInfo.at(segmentCounter))
                             .at(m_lastRepIndex)
                             .at(segmentCounter)) /
              bandwidth >
          (bufferNow - m_bufferMin) / 1000000.0) {
        int nextRepIndex;
        for (nextRepIndex = 0; nextRepIndex < m_lastRepIndex; nextRepIndex++) {
          if ((double)(8.0 * m_videoData.segmentSize
                                 .at(m_videoData.userInfo.at(segmentCounter))
                                 .at(nextRepIndex)
                                 .at(segmentCounter)) /
                  bandwidth >
              (bufferNow - m_bufferMin) / 1000000.0)
            break;
        }
        answer.nextRepIndex = nextRepIndex > 0 ? nextRepIndex - 1 : 0;
        answer.decisionCase = 3;
      } else if (bufferNow <= m_bufferLow) {
        if (m_lastRepIndex < m_highestRepIndex &&
            (double)(8.0 * m_videoData.segmentSize
                               .at(m_videoData.userInfo.at(segmentCounter))
                               .at(m_lastRepIndex + 1)
                               .at(segmentCounter)) /
                    bandwidth <=
                (bufferNow - m_bufferMin) / 1000000.0) {
          answer.nextRepIndex = m_lastRepIndex + 1;
          answer.decisionCase = 4;
        } else {
          answer.nextRepIndex = m_lastRepIndex;
          answer.decisionCase = 5;
        }
      } else if (bufferNow <= m_bufferHigh) {
        int nextRepIndex;
        for (nextRepIndex = m_highestRepIndex; nextRepIndex >= m_lastRepIndex;
             nextRepIndex--) {
          if ((double)(8.0 * m_videoData.segmentSize
                                 .at(m_videoData.userInfo.at(segmentCounter))
                                 .at(nextRepIndex)
                                 .at(segmentCounter)) /
                  bandwidth <=
              (bufferNow - m_bufferMin) / 1000000.0)
            break;
        }
        answer.nextRepIndex = nextRepIndex;
        answer.decisionCase = 6;
      } else if (bufferNow > m_bufferHigh) {
        int nextRepIndex;
        for (nextRepIndex = m_highestRepIndex; nextRepIndex >= m_lastRepIndex;
             nextRepIndex--) {
          if ((double)(8.0 * m_videoData.segmentSize
                                 .at(m_videoData.userInfo.at(segmentCounter))
                                 .at(nextRepIndex)
                                 .at(segmentCounter)) /
                  bandwidth <=
              (bufferNow - m_bufferLow) / 1000000.0)
            break;
        }
        answer.nextRepIndex = nextRepIndex;
        answer.decisionCase = 6;
      } else {
        answer.nextRepIndex = m_lastRepIndex;
        answer.decisionCase = 2;
      }
    }

    if (bufferNow > m_bufferHigh) {
      answer.nextDownloadDelay = (int64_t)(bufferNow - m_bufferHigh);
      answer.delayDecisionCase = 1;
    }
  } else {
    answer.nextRepIndex = m_lastRepIndex;
    answer.decisionCase = 0;
  }

  // add for xhinaxobile
  if (m_bufferHigh > m_bufferUpperbound - m_videoData.segmentDuration) {
    answer.nextDownloadDelay +=
        m_bufferHigh - (m_bufferUpperbound - m_videoData.segmentDuration);
  }
  //

  answer.estimateTh = bandwidth;
  m_lastRepIndex = answer.nextRepIndex;

  return answer;
}

}  // namespace ns3