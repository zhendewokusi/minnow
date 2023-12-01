#include "tcp_sender.hh"
#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdio>
#include <functional>
#include <random>
#include <string>

using namespace std;
/*
- keep track receiver's window
- fill the window when possible
*/

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return outstanding_cnt_;
}
uint64_t TCPSender::consecutive_retransmissions() const
{
  return retransmission_cnt_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if ( queued_segments_.empty() ) {
    return {};
  }
  // 开计时器
  if ( !retimer_.is_running() ) {
    retimer_.start();
  }
  TCPSenderMessage msg = queued_segments_.front();
  queued_segments_.pop();

  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  // 如果窗口大小为0则设置为1
  size_t curr_window_size = window_size_ != 0 ? window_size_ : 1;
  // 从Reader流获取Message
  while ( outstanding_cnt_ < curr_window_size ) {
    TCPSenderMessage msg;
    if ( !is_SYN_ ) {
      is_SYN_ = msg.SYN = true;
      outstanding_cnt_++;
    }
    const auto payload_size = min( TCPConfig::MAX_PAYLOAD_SIZE, curr_window_size - outstanding_cnt_ );
    msg.seqno = Wrap32::wrap( next_seq_, isn_ );
    read( outbound_stream, payload_size, msg.payload );
    outstanding_cnt_ += msg.payload.size();

    // FIN判断应该在 msg.sequence_length() == 0 前面 send_window.cc
    if (!is_FIN_ && outbound_stream.is_finished() && outstanding_cnt_ < curr_window_size) {
      is_FIN_ = msg.FIN = true;
      outstanding_cnt_++;
    }

    if ( msg.sequence_length() == 0 ) {
      break;
    }

    queued_segments_.push( msg );
    next_seq_ += msg.sequence_length();
    outstanding_segments_.push( msg );

    if (msg.FIN || outbound_stream.bytes_buffered() == 0) {
      break;
    }
  }
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
    auto tmp_ack_seq_ = msg.ackno.value().unwrap( Wrap32( isn_ ), next_seq_ );
    if(tmp_ack_seq_ > next_seq_)  {return;}
    ack_seq_ = tmp_ack_seq_;
  }
  while ( !outstanding_segments_.empty() ) {
    auto front_msg = outstanding_segments_.front();
    if ( front_msg.seqno.unwrap( isn_, next_seq_ ) + front_msg.sequence_length() <= ack_seq_ ) {

      outstanding_cnt_ -= front_msg.sequence_length();
      outstanding_segments_.pop();
      retimer_.RTO_reset();
      retransmission_cnt_ = 0;  // 接收到消息后应该将重传次数归零，和后续消息无关 send_retx.cc
      if (!outstanding_segments_.empty()) {
        retimer_.start();
      }
    } else {
      break;
    }
  }
  if (outstanding_segments_.empty()) {
    retimer_.stop();
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  retimer_.tick( ms_since_last_tick );
  if (retimer_.is_timeout()) {
    queued_segments_.push(outstanding_segments_.front());
    retimer_.RTO_double();
    retransmission_cnt_++;
    retimer_.start();
  }
}