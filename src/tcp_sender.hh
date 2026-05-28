#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

class TCPSender
{
  // 初始序列号 ISN
  Wrap32 isn_;
  // 初始重传超时
  uint64_t initial_RTO_ms_;
  // 已经确认到的“绝对序列号”位置。
  uint64_t ackno = 0;
  // 下一个要使用的“绝对序列号”。
  uint64_t seqno = 0;
  // 接收端窗口大小
  uint16_t window_size = 1;
  // 当前逻辑时钟
  uint64_t time_point = 0;
  // 当前重传超时
  uint64_t rto;
  // 连续重传次数
  uint64_t failed_cnt = 0;
  // 最早未确认报文的计时起点
  uint64_t start_time = 0;
  // FIN 是否已经发出
  bool fin_send = false;
  // 待发送队列，maybe_send() 从这里拿包发出去
  std::queue<std::shared_ptr<TCPSenderMessage>> mq {};
  // 已发送但未确认队列，用来跟踪重传
  std::queue<std::shared_ptr<TCPSenderMessage>> wq {};

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
