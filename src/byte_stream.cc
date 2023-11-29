#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), queue(), push_end( false ), push_error( false ), push_size( 0 ), pop_size( 0 )
{
  // 构造函数的函数体
  // ...
}

void Writer::push( string data )
{
  uint64_t len = min( data.size(), capacity_ - queue.size() );
  for ( uint64_t i = 0; i < len; i++ ) {
    queue.push( data[i] );
    push_size++;
  }
}

void Writer::close()
{
  push_end = true;
}
// what's up ?
void Writer::set_error()
{
  push_error = true;
}

bool Writer::is_closed() const
{
  return push_end;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - queue.size();
}

uint64_t Writer::bytes_pushed() const
{
  return push_size;
}

std::string_view Reader::peek() const
{
  if ( queue.empty() )
    return {};
  std::string_view view( &queue.front(), 1 );
  return view;
}
bool Reader::is_finished() const
{
  return queue.empty() && push_end;
}

bool Reader::has_error() const
{
  return push_size != pop_size || push_error;
}

void Reader::pop( uint64_t len )
{
  uint64_t pop_len = min( len, queue.size() );
  for ( uint64_t i = 0; i < pop_len; i++ ) {
    queue.pop();
    pop_size++;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return queue.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return pop_size;
}
