#include "tcp_receiver.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <iostream>
#include <optional>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if ( !zero_point_set ) {
    if ( !message.SYN ) {
      return;
    }
    zero_point_set = true;
    zero_point = message.seqno;
  }
  // 最接近的下一个 TCP 绝对序号位置
  const uint64_t checkpoint = inbound_stream.bytes_pushed() + 1;
  // 得到绝对地址
  const uint64_t absseq = message.seqno.unwrap( zero_point, checkpoint );

  uint64_t stream_index = 0;
  if ( message.SYN ) {
    stream_index = 0;
  } else {
    // SYN在绝对地址里占一个位置，映射到流下标时，就要减 1
    if ( absseq == 0 ) {
      return;
    }
    stream_index = absseq - 1;
  }

  reassembler.insert( stream_index, message.payload, message.FIN, inbound_stream );
  // 下一个还期望收到的 TCP 序号
  ackno = inbound_stream.bytes_pushed() + 1;
  // FIN
  if ( inbound_stream.is_closed() ) {
    ackno += 1;
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  // 当前接收缓冲区还能装多少数据
  uint64_t capacity = inbound_stream.available_capacity();
  // 剩余容量
  uint16_t window_size = static_cast<uint16_t>( std::min( capacity, static_cast<uint64_t>( UINT16_MAX ) ) );

  if ( !zero_point_set ) {
    return { std::nullopt, window_size };
  }
  // 包装下一个还期望收到的 TCP 序号
  return { Wrap32::wrap( ackno, zero_point ), window_size };
}
