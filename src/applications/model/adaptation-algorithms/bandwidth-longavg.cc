#include "bandwidth-longavg.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BandwidthLongAvgAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(BandwidthLongAvgAlgorithm);

BandwidthLongAvgAlgorithm::BandwidthLongAvgAlgorithm(
    const videoData &videoData, const playbackData &playbackData,
    const bufferData &bufferData, const throughputData &throughput)
    : BandwidthAlgorithm(videoData, playbackData, bufferData, throughput),
      m_lastBandwidthEstimate(0.0),  // default bandwidthEstimate value = 0
      m_highestRepIndex(videoData.averageBitrate[0].size() - 1) {
  NS_LOG_INFO(this);
  NS_ASSERT_MSG(m_highestRepIndex >= 0,
                "The highest quality representation index should be >= 0");
}

bandwidthAlgoReply BandwidthLongAvgAlgorithm::BandwidthAlgo(
    const int64_t segmentCounter,
    const int64_t clientId)  // reservation
{
  bandwidthAlgoReply answer;

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
    if (segmentCounter != 1) {
      m_lastBandwidthEstimate =
          m_lastBandwidthEstimate +
          (lastSegmentThroughput - m_lastBandwidthEstimate) / segmentCounter;
      answer.decisionCase = 2;
    } else {
      m_lastBandwidthEstimate = lastSegmentThroughput;
      answer.decisionCase = 1;
    }
  } else {
    m_lastBandwidthEstimate = m_lastBandwidthEstimate;
    answer.decisionCase = 0;
  }
  answer.bandwidthEstimate = m_lastBandwidthEstimate;
  return answer;
}

}  // namespace ns3
