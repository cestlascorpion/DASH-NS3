#ifndef TOMATO_ALGORITHM_H
#define TOMATO_ALGORITHM_H

#include <algorithm>
#include <deque>
#include <vector>
#include "tcp-stream-adaptation.h"

namespace ns3 {
// designed by tian
class TomatoAlgorithm : public AdaptationAlgorithm {
 public:
  TomatoAlgorithm(const videoData &videoData, const playbackData &playbackData,
                  const bufferData &bufferData,
                  const throughputData &throughput);

  algorithmReply GetNextRep(const int64_t segmentCounter,
                            const int64_t clientId, int64_t bandwidth);

 private:
  int64_t m_lastRepIndex;
  int64_t m_targetBuffer;
  int64_t m_bufferMin;
  int64_t m_expBuffer;
  int64_t m_multipleTinyDrop;
  double m_beta;
  const int64_t m_bufferUpperbound;
  std::deque<int> m_recentRepIndex;
  const int64_t m_maxQueueSize;
  const int64_t m_highestRepIndex;
};

}  // namespace ns3
#endif /* TOMATO_ALGORITHM_H */
