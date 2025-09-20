#pragma once
#include <vector>
#include <string>
#include "byte_stream.hh"

using namespace std;
class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ),
                                                next_idx_( 0 ),
                                                bytes_pending_( 0 ),
                                                bytes_total_( 0 ),
                                                eof_set_( false ),
                                                rs_buffer_(""),
                                                rs_marked_(""){
                                                  int rs_cap = output_.writer().available_capacity();
                                                  for(int i=0;i<rs_cap;i++){
                                                    rs_buffer_ +=' ';
                                                    rs_marked_ +='0';
                                                  }
                                                }

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_; // the Reassembler writes to this ByteStream
  uint64_t next_idx_;
  uint64_t bytes_pending_;
  uint64_t bytes_total_;
  bool eof_set_;
  string rs_buffer_;
  string rs_marked_;
};
