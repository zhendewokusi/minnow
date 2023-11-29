#include "tcp_sender.hh"
#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <random>
#include <string>
#include <cstdio>

using namespace std;
/*
- keep track receiver's window
- fill the window when possible
*/

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms ){}

uint64_t TCPSender::sequence_numbers_in_flight() const {return outstanding_cnt_;}
uint64_t TCPSender::consecutive_retransmissions() const {return retransmission_cnt_;}


optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if(queued_segments_.empty()) {
    return {};
  }
  // 开计时器
  if(!retimer_.is_running()) {
    retimer_.start();
  }
  auto& msg = queued_segments_.front();
  queued_segments_.pop();

  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  // 如果窗口大小为0则设置为1
  size_t curr_window_size = window_size_ != 0 ? window_size_ : 1;
  // 从Reader流获取Message
  TCPSenderMessage msg;
  if(!is_SYN_){
    is_SYN_ = msg.SYN = true;
    outstanding_cnt_++;
  }
  const auto payload_size = min( TCPConfig::MAX_PAYLOAD_SIZE, curr_window_size - outstanding_cnt_ );
  msg.seqno = Wrap32::wrap(next_seq_, isn_);
  read(outbound_stream,payload_size,msg.payload);
  outstanding_cnt_ += payload_size;
  
  queued_segments_.push(msg);
  next_seq_ += msg.sequence_length();
  outstanding_segments_.push(msg);
}

// 发送空消息
TCPSenderMessage TCPSender::send_empty_message() const
{
  auto seqno = Wrap32::wrap( next_seq_, Wrap32( isn_ ) );
  return { seqno, false, {}, false };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  window_size_ = msg.window_size;
  if ( msg.ackno.has_value() ) {
    ack_seq_ = msg.ackno.value().unwrap( Wrap32( isn_ ), next_seq_ );
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  retimer_.tick(ms_since_last_tick);
}