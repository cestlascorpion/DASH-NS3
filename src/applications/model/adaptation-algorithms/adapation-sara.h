#ifndef SARA_ALGORITHM_H
#define SARA_ALGORITHM_H

#include "tcp-stream-adaptation.h"

namespace ns3 {
// do not use this algo
class SaraAlgorithm : public AdaptationAlgorithm {
 public:
  SaraAlgorithm(const videoData &videoData, const playbackData &playbackData,
                const bufferData &bufferData, const throughputData &throughput);

  algorithmReply GetNextRep(const int64_t segmentCounter,
                            const int64_t clientId, int64_t bandwidth);

 private:
  int64_t m_lastRepIndex;
  int64_t m_bufferHigh;
  int64_t m_bufferLow;
  int64_t m_bufferMin;
  const int64_t m_bufferUpperbound;
  const int64_t m_highestRepIndex;
};

}  // namespace ns3
#endif /* SARA_ALGORITHM_H */
