#include "adapation-constbitrate.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("constbitrateAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(constbitrateAlgorithm);

constbitrateAlgorithm::constbitrateAlgorithm(const videoData &videoData,
                                             const playbackData &playbackData,
                                             const bufferData &bufferData,
                                             const throughputData &throughput)
    : AdaptationAlgorithm(videoData, playbackData, bufferData, throughput),
      m_constRepIndex(4),                                    // const repIndex
      m_bufferUpperbound(m_videoData.segmentDuration * 15),  // upper bound
      m_highestRepIndex(videoData.averageBitrate[0].size() - 1) {
  NS_LOG_INFO(this);
  NS_ASSERT_MSG(m_highestRepIndex >= 0,
                "The highest quality representation index should be => 0");
}

algorithmReply constbitrateAlgorithm::GetNextRep(const int64_t segmentCounter,
                                                 int64_t clientId,
                                                 int64_t bandwidth) {
  algorithmReply answer;
  int64_t timeNow = Simulator::Now().GetMicroSeconds();
  answer.decisionTime = timeNow;
  answer.nextRepIndex = m_constRepIndex;
  answer.decisionCase = 0;
  answer.nextDownloadDelay = 0;
  answer.delayDecisionCase = 0;
  answer.estimateTh = bandwidth;

  // add for xhinaxobile
  if (segmentCounter != 0) {
    int64_t bufferNow = m_bufferData.bufferLevelNew.back() -
                        (timeNow - m_throughput.transmissionEnd.back());
    if (bufferNow > m_bufferUpperbound - m_videoData.segmentDuration) {
      answer.nextDownloadDelay +=
          bufferNow - (m_bufferUpperbound - m_videoData.segmentDuration);
    }
  }
  //
  return answer;
}
}  // namespace ns3
