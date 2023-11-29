#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // 设置SYN标识，表示来的包是合法的
  if ( message.SYN ) {
    is_syn = true;
    zero_point = message.seqno;
  } else if ( !is_syn ) {
    // 非法数据包
    return;
  }
  // Write 总共接收的 bytes 数，作为绝对序列号的checkpoint
  uint64_t checkpoint = inbound_stream.bytes_pushed();
  uint64_t seq = Wrap32( message.seqno ).unwrap( zero_point, checkpoint );
  // 可能来的FIN包是非法的，因此不能将其防止在上面包的 if 判断中
  if ( message.FIN ) {
    is_fin = true;
  }
  // 这里 -1 是因为 reassembler 的 unassembled_index_ 是从 0 开始的，如果加 SYN 就从 1 开始传参，不能将其重组
  reassembler.insert( seq + message.SYN - 1, message.payload, is_fin, inbound_stream );
  // reassembler.insert(seq + is_syn - 1,message.payload,is_fin,inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage receivermessage;
  receivermessage.window_size
    = inbound_stream.available_capacity() > UINT16_MAX ? UINT16_MAX : inbound_stream.available_capacity();

  uint64_t num_bytes = inbound_stream.bytes_pushed();
  if ( is_syn ) {
    if ( is_fin && inbound_stream.is_closed() ) {
      num_bytes++;
    }
    receivermessage.ackno = Wrap32::wrap( num_bytes + 1, zero_point );
  }

  return receivermessage;
}