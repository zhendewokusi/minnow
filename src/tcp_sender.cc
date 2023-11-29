#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "wrapping_integers.hh"

#include <random>

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
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  outbound_stream.bytes_buffered();
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
  // Your code here.
  (void)ms_since_last_tick;
}
