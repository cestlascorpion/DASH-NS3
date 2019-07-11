#ifndef BANDWIDTHAVGINTIME_ALGORITHM_H
#define BANDWIDTHAVGINTIME_ALGORITHM_H
#include "tcp-stream-bandwidth.h"

namespace ns3 {

class BandwidthAvgInTimeAlgorithm : public BandwidthAlgorithm {
 public:
  BandwidthAvgInTimeAlgorithm(const videoData &videoData,
                              const playbackData &playbackData,
                              const bufferData &bufferData,
                              const throughputData &throughput);

  bandwidthAlgoReply BandwidthAlgo(const int64_t segmentCounter,
                                   const int64_t clientId);

 private:
  double AverageBandwidth(int64_t t1, int64_t t2, int64_t &decisioncase);

  int64_t m_deltaTime;
  double m_lastBandwidthEstimate;  // Last bandwidthEstimate Value
  const int64_t m_highestRepIndex;
};

}  // namespace ns3
#endif /* BANDWIDTHAVGINTIME_ALGORITHM_H */