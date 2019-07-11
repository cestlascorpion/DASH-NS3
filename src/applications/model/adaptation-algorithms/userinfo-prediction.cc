#include "userinfo-prediction.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("UserPredictionAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(UserPredictionAlgorithm);

UserPredictionAlgorithm::UserPredictionAlgorithm(
    const videoData &videoData, const playbackData &playbackData,
    const bufferData &bufferData, const throughputData &throughput)
    : UserinfoAlgorithm(videoData, playbackData, bufferData, throughput) {
  NS_LOG_INFO(this);
}

userinfoAlgoReply UserPredictionAlgorithm::UserinfoAlgo(
    const int64_t segmentCounter, const int64_t clientId) {
  userinfoAlgoReply answer;
  const int64_t timeNow = Simulator::Now().GetMicroSeconds();
  answer.decisionTime = timeNow;
  return answer;
}

}  // namespace ns3