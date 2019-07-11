#ifndef SEGMENTAWARE_ALGORITHM_H
#define SEGMENTAWARE_ALGORITHM_H

#include <queue>
#include "tcp-stream-adaptation.h"

namespace ns3 {
// designed by tian
class SegmentAwareAlgorithm : public AdaptationAlgorithm {
 public:
  SegmentAwareAlgorithm(const videoData &videoData,
                        const playbackData &playbackData,
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
  std::deque<int64_t> m_recentRepIndex;
  const int64_t m_maxQueueSize;
  const int64_t m_highestRepIndex;
};

}  // namespace ns3

#endif  // !SEGMENTAWARE_ALGORITHM_H