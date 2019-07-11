#ifndef CONSTBITRATE_ALGORITHM_H
#define CONSTBITRATE_ALGORITHM_H

#include "tcp-stream-adaptation.h"

namespace ns3 {

class constbitrateAlgorithm : public AdaptationAlgorithm {
 public:
  constbitrateAlgorithm(const videoData &videoData,
                        const playbackData &playbackData,
                        const bufferData &bufferData,
                        const throughputData &throughput);

  algorithmReply GetNextRep(const int64_t segmentCounter,
                            const int64_t clientId, int64_t bandwidth);

 private:
  const int64_t m_constRepIndex;
  const int64_t m_bufferUpperbound;
  const int64_t m_highestRepIndex;
};

}  // namespace ns3
#endif /* CONSTBITRATE_ALGORITHM_H */