#include "tcp-stream-bandwidth.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BandwidthAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(BandwidthAlgorithm);

BandwidthAlgorithm::BandwidthAlgorithm(const videoData &videoData,
                                       const playbackData &playbackData,
                                       const bufferData &bufferData,
                                       const throughputData &throughput)
    : m_videoData(videoData),
      m_bufferData(bufferData),
      m_throughput(throughput),
      m_playbackData(playbackData) {}

}  // namespace ns3