/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2016 Technische Universitaet Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef TCP_STREAM_INTERFACE_H
#define TCP_STREAM_INTERFACE_H

namespace ns3 {

/*! \class algorithmReply tcp-stream-interface.h "model/tcp-stream-interface.h"
 *  \ingroup tcpStream
 *  \brief This struct contains the reply an adaptation algorithm returns to the
 * client
 *
 * Data structure that an adaptation algorithm returns to the client, containing
 * all the information the client needs for deciding which representation index
 * to request from the server next, and when to schedule the next request in
 * microseconds. Optionally the following variables can be used for logging:
 * when the algorithm made the decision and at which part of the code it decided
 * which representation index and inter-request time to select.
 */
struct algorithmReply {
  int64_t nextRepIndex;
  int64_t nextDownloadDelay;
  int64_t decisionTime;
  int64_t decisionCase;
  int64_t delayDecisionCase;
  double estimateTh;
};

struct bandwidthAlgoReply {
  double bandwidthEstimate;
  int64_t decisionCase;
};
struct userinfoAlgoReply {
  int64_t decisionTime;
};
/*! \class throughputData tcp-stream-interface.h "model/tcp-stream-interface.h"
 *  \ingroup tcpStream
 *  \brief This is a struct containing throughput data.
 *
 * Contains throughput data that the adaptation algorithm is provided by the
 * client. These values are needed to compute the next representation index
 * based on previous throughput.
 */
struct throughputData {
  std::vector<int64_t>
      transmissionRequested;  //!< Simulation time in microseconds when a
                              //!< segment was requested by the client
  std::vector<int64_t>
      transmissionStart;  //!< Simulation time in microseconds when the first
                          //!< packet of a segment was received
  std::vector<int64_t>
      transmissionEnd;  //!< Simulation time in microseconds when the last
                        //!< packet of a segment was received
  std::vector<int64_t>
      bytesReceived;  //!< Number of bytes received, i.e. segment size
};

/*! \class bufferData tcp-stream-interface.h "model/tcp-stream-interface.h"
 *  \ingroup tcpStream
 *  \brief This is a struct containing buffer data.
 *  \usually Only bufferData is used for dash, considering layer-based video
 * coding, enhancement layer buffer may be required Tracks the status of the
 * buffer level.
 */
struct bufferData {
  std::vector<int64_t> timeNow;       //!< current simulation time
  std::vector<int64_t> segmentIndex;  //!< segmentIndex of segment in
                                      //!< buffer(base layer) -1 = fail
  std::vector<int64_t>
      bufferLevelOld;  //!< buffer level in microseconds before adding segment
                       //!< duration (in microseconds) of just downloaded
                       //!< segment
  std::vector<int64_t>
      bufferLevelNew;  //!< buffer level in microseconds after adding segment
                       //!< duration (in microseconds) of just downloaded
                       //!< segment
};

/*! \class videoData tcp-stream-interface.h "model/tcp-stream-interface.h"
 *  \ingroup tcpStream
 *  \brief This is a struct containing video data.
 *
 * Reduced version of a MPEG-DASH Media Presentation Description (MPD),
 * containing a 2D [i][j] matrix containing the size of every segment j in
 * representation level i, the average bitrate of every representation level and
 * the duration of a segment in microseconds.
 */
struct videoData {
  std::vector<std::vector<std::vector<int64_t>>>
      segmentSize;  //!< vector holding representation levels in the first
                    //!< dimension and their particular segment sizes in bytes
                    //!< in the second dimension
                    // 2-D vector, < viewPoint < RepsLevel <segmentsize>>
  std::vector<std::vector<double>>
      averageBitrate;       //!< holding the average bitrate of a segment in
                            //!< representation i in bits
                            // 2-D vector, < viewPoint < RepsLevel's Bitrate >
  int64_t segmentDuration;  //!< duration of a segment in microseconds
  std::vector<int64_t> repIndex;  // repIndex
  std::vector<int64_t> userInfo;  // userViewPoint
};

/*! \class playbackData tcp-stream-interface.h "model/tcp-stream-interface.h"
 *  \ingroup tcpStream
 *  \brief This is a struct containing playback data.
 *
 * A pair of values, playbackIndex representing the index of a segment:
 * multiplication with segmentDuration yields the point in time in microseconds
 * where on the timeline of the video file the start of this segment can be
 * found. playbackStart however, indicates the start of the playback (in the
 * streaming process) of the segment in microseconds in simulation time.
 */
struct playbackData {
  std::vector<int64_t> playbackIndex;  //!< Index of the video segment
  std::vector<int64_t> playbackStart;  //!< Point in time in microseconds when
                                       //!< playback of this segment started
};

}  // namespace ns3

#endif /* TCP_STREAM_CLIENT_H */
