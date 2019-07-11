#include "bandwidth-wharmonic.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BandwidthWHarmonicAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(BandwidthWHarmonicAlgorithm);

BandwidthWHarmonicAlgorithm::BandwidthWHarmonicAlgorithm(
    const videoData &videoData, const playbackData &playbackData,
    const bufferData &bufferData, const throughputData &throughput)
    : BandwidthAlgorithm(videoData, playbackData, bufferData, throughput),
      m_windowSize(5),  // 5 segment for smoothing
      m_highestRepIndex(videoData.averageBitrate[0].size() - 1) {
  NS_LOG_INFO(this);
  NS_ASSERT_MSG(m_highestRepIndex >= 0,
                "The highest quality representation index should be >= 0");
}

bandwidthAlgoReply BandwidthWHarmonicAlgorithm::BandwidthAlgo(
    const int64_t segmentCounter, const int64_t clientId) {
  bandwidthAlgoReply answer;

  double bandwidthEstimate = 0.0;

  if (segmentCounter != 0) {
    int64_t sumThroughput = 0;  // bit
    double transmissionTime = 0.0;
    if (segmentCounter < m_windowSize)  // 5
    {
      for (int64_t i = 0; i != segmentCounter; i++) {
        sumThroughput +=
            8 * m_videoData.segmentSize.at(m_videoData.userInfo.at(i))
                    .at(m_videoData.repIndex.at(i))
                    .at(i);
        transmissionTime += (m_throughput.transmissionEnd.at(i) -
                             m_throughput.transmissionStart.at(i));
      }
      bandwidthEstimate =
          (double)sumThroughput * 1000000.0 / (transmissionTime);
      answer.decisionCase = 1;
    } else {
      for (int64_t i = segmentCounter - m_windowSize; i != segmentCounter;
           i++) {
        sumThroughput +=
            8 * m_videoData.segmentSize.at(m_videoData.userInfo.at(i))
                    .at(m_videoData.repIndex.at(i))
                    .at(i);
        transmissionTime += (m_throughput.transmissionEnd.at(i) -
                             m_throughput.transmissionStart.at(i));
      }
      bandwidthEstimate = (double)sumThroughput * 1000000.0 / transmissionTime;
      answer.decisionCase = 2;
    }
  } else {
    answer.bandwidthEstimate = bandwidthEstimate;
    answer.decisionCase = 0;
  }
  answer.bandwidthEstimate = bandwidthEstimate;
  return answer;
}

}  // namespace ns3