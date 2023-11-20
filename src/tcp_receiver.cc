#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "wrapping_integers.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  
  // message.seqno
  (void)message;
  (void)reassembler;
  (void)inbound_stream;
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage receivermessage;
  // Wrap32 wrap;
  receivermessage.window_size =  inbound_stream.available_capacity();
  return receivermessage;
}
