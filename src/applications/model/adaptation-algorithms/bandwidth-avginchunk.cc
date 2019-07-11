#include "bandwidth-avginchunk.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BandwidthAvgInChunkAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(BandwidthAvgInChunkAlgorithm);

BandwidthAvgInChunkAlgorithm::BandwidthAvgInChunkAlgorithm(
    const videoData &videoData, const playbackData &playbackData,
    const bufferData &bufferData, const throughputData &throughput)
    : BandwidthAlgorithm(videoData, playbackData, bufferData, throughput),
      m_windowSize(5),  // 5 segment fot smoothing
      m_highestRepIndex(videoData.averageBitrate[0].size() - 1) {
  NS_LOG_INFO(this);
  NS_ASSERT_MSG(m_highestRepIndex >= 0,
                "The highest quality representation index should be >= 0");
}

bandwidthAlgoReply BandwidthAvgInChunkAlgorithm::BandwidthAlgo(
    const int64_t segmentCounter, const int64_t clientId) {
  bandwidthAlgoReply answer;
  double bandwidthEstimate = 0.0;

  if (segmentCounter != 0) {
    double lastSegmentDownloadTime =
        (double)(m_throughput.transmissionEnd.at(segmentCounter - 1) -
                 m_throughput.transmissionStart.at(segmentCounter - 1)) /
        1000000.0;
    double lastSegmentThroughput =
        8.0 *
        m_videoData.segmentSize.at(m_videoData.userInfo.at(segmentCounter - 1))
            .at(m_videoData.repIndex.at(segmentCounter - 1))
            .at(segmentCounter - 1) /
        lastSegmentDownloadTime;
    m_lastBandwidthEstimate.push_back(lastSegmentThroughput);
    int64_t windowSize = m_lastBandwidthEstimate.size();
    double sumDownloadRate = 0.0;
    if (windowSize < m_windowSize)  // 5
    {
      for (int64_t i = 0; i != windowSize; i++) {
        sumDownloadRate += m_lastBandwidthEstimate.at(i);
      }
      bandwidthEstimate = sumDownloadRate / (double)windowSize;
      answer.decisionCase = 1;
    } else {
      for (int64_t i = windowSize - m_windowSize; i != windowSize; i++) {
        sumDownloadRate += m_lastBandwidthEstimate.at(i);
      }
      bandwidthEstimate = sumDownloadRate / (double)m_windowSize;
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
