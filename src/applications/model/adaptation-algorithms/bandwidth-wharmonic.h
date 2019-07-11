#ifndef BANDWIDTHWHARMONIC_ALGORITHM_H
#define BANDWIDTHWHARMONIC_ALGORITHM_H
#include "tcp-stream-bandwidth.h"

namespace ns3 {

class BandwidthWHarmonicAlgorithm : public BandwidthAlgorithm {
 public:
  BandwidthWHarmonicAlgorithm(const videoData &videoData,
                              const playbackData &playbackData,
                              const bufferData &bufferData,
                              const throughputData &throughput);

  bandwidthAlgoReply BandwidthAlgo(const int64_t segmentCounter,
                                   const int64_t clientId);

 private:
  const int64_t m_windowSize;
  const int64_t m_highestRepIndex;
};

}  // namespace ns3
#endif /* BANDWIDTHWHARMONIC_ALGORITHM_H */
