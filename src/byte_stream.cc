#include <stdexcept>
#include <iostream>
#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), 
                                              buffer_(""),
                                              closed_(false),
                                              error_(false),
                                              bytes_pushed_( 0 ){}

void Writer::push( string data )
{
  // Your code here.
  uint64_t dlen = min(data.length(),capacity_-buffer_.length());
  buffer_ +=data.substr(0,dlen);
  bytes_pushed_ += dlen; 
}

void Writer::close()
{
  // Your code here.
  closed_=true;
}

void Writer::set_error()
{
  // Your code here.
  error_=true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_-buffer_.length();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  return buffer_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return buffer_.length()==0 && closed_;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  buffer_=buffer_.substr(len);
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.length();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_pushed_-buffer_.length();
}
