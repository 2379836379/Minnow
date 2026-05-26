#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), 
  //left读指针
  lpos(0),
  // right 写指针 
  rpos(0), buf(), is_closed_(false), error_(false) {
  buf.resize(capacity_);
}

void Writer::push( string data )
{
  // Your code here.
  //assert(data.size() + rpos - lpos <= capacity_);
  if(capacity_ == 0 || data.empty()){
    return;
  }
  uint64_t len = min<uint64_t>(available_capacity(), data.size());
  for(uint64_t i = 0; i < len; ++i){
    buf[(rpos)%capacity_] = data[i];
    rpos++;
  }
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - (rpos - lpos);
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return rpos;
}

string_view Reader::peek() const
{
  // Your code here.
  if (lpos == rpos) {
    return "";
  }
  uint64_t r = rpos % capacity_;
  uint64_t l = lpos % capacity_;
  if (l < r) {
    return {buf.data() + l, r - l};
  } else {
    //不考虑回绕部分
    return {buf.data() + l, capacity_ - l};
  }
}

bool Reader::is_finished() const
{
  // Your code here.
  return !error_ && is_closed_ && rpos == lpos;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  lpos += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return rpos - lpos;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return lpos;
}
