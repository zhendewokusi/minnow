#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "retran_timer.hh"

#include <cstdint>

class TCPSender
{
  bool is_SYN_ { false };
  bool is_FIN_ { false };

  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  uint64_t retransmission_cnt_ { 0 }; // 重传次数
  uint64_t outstanding_cnt_ { 0 };
  uint64_t window_size_ { 1 }; // 窗口大小
  uint64_t next_seq_ { 0 };    // 下一个序列号
  uint64_t ack_seq_ { 0 };     // 确认号
  ReTimer retimer_{initial_RTO_ms_};  // 定时器

  // 传输中
  std::queue<TCPSenderMessage> outstanding_segments_ {};
  // 需要发送
  std::queue<TCPSenderMessage> queued_segments_ {};
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
