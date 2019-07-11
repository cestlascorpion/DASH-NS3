#ifndef BANDWIDTHHARMONIC_ALGORITHM_H
#define BANDWIDTHHARMONIC_ALGORITHM_H
#include "tcp-stream-bandwidth.h"

namespace ns3 {

class BandwidthHarmonicAlgorithm : public BandwidthAlgorithm {
 public:
  BandwidthHarmonicAlgorithm(const videoData &videoData,
                             const playbackData &playbackData,
                             const bufferData &bufferData,
                             const throughputData &throughput);

  bandwidthAlgoReply BandwidthAlgo(const int64_t segmentCounter,
                                   const int64_t clientId);

 private:
  const int64_t m_windowSize;
  std::vector<double> m_lastBandwidthEstimate;
  const int64_t m_highestRepIndex;
};

}  // namespace ns3
#endif /* BANDWIDTHHARMONIC_ALGORITHM_H */
