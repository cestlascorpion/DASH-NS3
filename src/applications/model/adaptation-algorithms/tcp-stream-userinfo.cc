#include "tcp-stream-userinfo.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("UserinfoAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(UserinfoAlgorithm);

UserinfoAlgorithm::UserinfoAlgorithm(const videoData &videoData,
                                     const playbackData &playbackData,
                                     const bufferData &bufferData,
                                     const throughputData &throughput)
    : m_videoData(videoData),
      m_bufferData(bufferData),
      m_throughput(throughput),
      m_playbackData(playbackData) {}

}  // namespace ns3