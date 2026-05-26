#include "reassembler.hh"

#include <algorithm>
#include <cstdint>
#include <string>

using namespace std;

void Reassembler::pop( Writer& output )
{
  const uint64_t limit = min( output.available_capacity(), bytes_pending_ );
  if ( limit == 0 ) {
    if ( endpos_set && next_expected == endpos ) {
      output.close();
    }
    return;
  }

  string assembled {};
  assembled.reserve( limit );

  while ( assembled.size() < limit ) {
    const uint64_t slot = next_expected % capacity_;
    if ( !is_value[slot] ) {
      break;
    }

    assembled.push_back( buf[slot] );
    is_value[slot] = false;
    ++next_expected;
    --bytes_pending_;
  }

  if ( !assembled.empty() ) {
    output.push( std::move( assembled ) );
  }

  if ( endpos_set && next_expected == endpos ) {
    output.close();
  }
}

void Reassembler::insert( uint64_t first_index, string data_in, bool is_last_substring, Writer& output )
{
  if ( capacity_ == 0 ) {
    // 初始容量为接收端内部 ByteStream 的写接口容量
    capacity_ = output.available_capacity();
    buf.resize( capacity_ );
    is_value.resize( capacity_, false );
  }

  if ( is_last_substring ) {
    endpos_set = true;
    endpos = first_index + data_in.size();
  }

  const uint64_t window_begin = next_expected;
  const uint64_t window_end = next_expected + output.available_capacity();
  const uint64_t data_end = first_index + data_in.size();
  const uint64_t insert_begin = max( first_index, window_begin );
  const uint64_t insert_end = min( data_end, window_end );

  for ( uint64_t idx = insert_begin; idx < insert_end; ++idx ) {
    const uint64_t slot = idx % capacity_;
    if ( !is_value[slot] ) {
      is_value[slot] = true;
      buf[slot] = data_in[idx - first_index];
      ++bytes_pending_;
    }
  }

  pop( output );
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}
