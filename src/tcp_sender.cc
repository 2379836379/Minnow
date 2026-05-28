#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"

#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <string_view>
#include <utility>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
// 初始重传超时时间和序列起始序号
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , rto( initial_RTO_ms )
{}
// 已发送但还未收到 ACK 确认的字节数
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return seqno - ackno;
}
// 发送端连续重传的次数
uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return failed_cnt;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  while ( !mq.empty() ) {
    auto msgp = mq.front();
    mq.pop();
    // 是否有值大于ackno
    if ( msgp->seqno.unwrap( isn_, ackno ) + msgp->sequence_length() > ackno ) {
      return *msgp;
    }
  }
  return std::nullopt;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  do {
    string m;
    uint64_t len = window_size;
    // TCP 在窗口为 0 时，发送方不能无限沉默，通常仍会尝试发一个很小的探测段，看看窗口是否重新打开
    if ( len < 1 ) {
      len = 1;
    }
    if ( len < seqno - ackno ) {
      len = 0;
    } else {
      // 减掉当前已经在飞的字节数
      len -= seqno - ackno;
    }
    // 是否因为报文大小上限中断，用在 FIN 的判断
    bool cantake = false;
    // 单个报文最多装多少 payload
    if ( len > TCPConfig::MAX_PAYLOAD_SIZE ) {
      len = TCPConfig::MAX_PAYLOAD_SIZE;
      cantake = true;
    }
    // 拼接
    while ( len ) {
      string_view sv = outbound_stream.peek();
      if ( sv.empty() ) {
        break;
      }
      if ( sv.length() >= len ) {
        m += sv.substr( 0, len );
        outbound_stream.pop( len );
        len = 0;
      } else {
        m += sv;
        len -= sv.length();
        outbound_stream.pop( sv.length() );
      }
    }
    bool syn = seqno == 0;
    // 只有窗口里还有空间时，才能把 FIN 放进去
    bool fin = !fin_send && outbound_stream.is_finished() && ( len > 0 || cantake );
    if ( !syn && !fin && m.empty() ) {
      return;
    }
    auto sp = make_shared<TCPSenderMessage>( Wrap32::wrap( seqno, isn_ ), syn, m, fin );
    mq.push( sp );
    if ( wq.empty() ) {
      start_time = time_point;
    }
    wq.push( sp );
    seqno += syn + m.size() + ( fin && !fin_send );
    fin_send |= fin;
    // 窗口还有剩余空间
  } while ( window_size > seqno - ackno && !outbound_stream.peek().empty() );
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  return { Wrap32::wrap( seqno, isn_ ), false, {}, false };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  uint64_t ack = msg.ackno.value_or( isn_ ).unwrap( isn_, ackno );
  // seqno 是发送端目前已经发送到的最远位置，判断ack是否合法
  // reassembler保证不会产生空缺，收到的内容会保留
  if ( ack <= seqno ) {
    if ( ack > ackno ) {
      ackno = ack;
      rto = initial_RTO_ms_;
      failed_cnt = 0;
      start_time = time_point;
    }
  }
  window_size = msg.window_size;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  time_point += ms_since_last_tick;
  while ( !wq.empty() && wq.front()->seqno.unwrap( isn_, ackno ) + wq.front()->sequence_length() <= ackno ) {
    wq.pop();
  }
  if ( !wq.empty() && time_point >= start_time + rto ) {
    auto sp = wq.front();
    mq.push( sp );
    start_time = time_point;
    if ( window_size ) {
      failed_cnt += 1;
      rto += rto;
    }
  }
}
