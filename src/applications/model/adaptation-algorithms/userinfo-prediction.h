#ifndef USERPREDICTION_ALGORITHM_H
#define USERPREDICTION_ALGORITHM_H
#include "tcp-stream-userinfo.h"

namespace ns3 {
// todo
class UserPredictionAlgorithm : public UserinfoAlgorithm {
 public:
  UserPredictionAlgorithm(const videoData &videoData,
                          const playbackData &playbackData,
                          const bufferData &bufferData,
                          const throughputData &throughput);

  userinfoAlgoReply UserinfoAlgo(const int64_t segmentCounter,
                                 const int64_t clientId);
};

}  // namespace ns3
#endif /* BUFFERCLEAN_ALGORITHM_H */