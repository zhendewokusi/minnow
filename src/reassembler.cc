#include "reassembler.hh"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;

bool Reassembler::is_closed() const
{
  return closed_ && bytes_pending() == 0;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if ( is_last_substring ) {
    closed_ = true;
  }

  if ( first_index >= unassembled_index_ + output.available_capacity() ||
       first_index + data.length() - 1 < unassembled_index_ ||
       data.empty() ||
       output.available_capacity() == 0 ) {
    if ( is_closed() ) {
      output.close();
    }
    return;
  }

  const uint64_t cap = output.available_capacity();

  uint64_t new_index = first_index;
  // 左边边界处理
  if ( first_index <= unassembled_index_ ) {
    new_index = unassembled_index_;
    const uint64_t overlapped_length = unassembled_index_ - first_index;
    data = data.substr( overlapped_length, min( data.size() - overlapped_length, cap ) );
  } else {
  // 右边边界处理
    data = data.substr( 0, min( data.size(), cap ) );
    if ( first_index + data.size() - 1 > unassembled_index_ + cap - 1 ) {
      data = data.substr( 0, unassembled_index_ + cap - first_index );
    }
  }
  //内部调整
  auto rear_iter = unassembled_substrings_.lower_bound( new_index );
  while ( rear_iter != unassembled_substrings_.end() ) {
    auto& [rear_index, rear_data] = *rear_iter;
    // 无重复部分
    if ( new_index + data.size() - 1 < rear_index ) {
      break;
    }
    uint64_t rear_overlapped_length = 0;
    if ( new_index + data.size() - 1 < rear_index + rear_data.size() - 1 ) {
      rear_overlapped_length = new_index + data.size() - rear_index;
    } else {
      rear_overlapped_length = rear_data.size();
    }

    const uint64_t next_rear = rear_index + rear_data.size() - 1;
    // 如果data完全包含之前存在的元素，将包含的元素删除
    if ( rear_overlapped_length == rear_data.size() ) {
      unassembled_bytes_ -= rear_data.size();
      unassembled_substrings_.erase( rear_index );
    } else {
    // 如果部分重合则删除data中的重合部分
      data.erase( data.end() - static_cast<int64_t>( rear_overlapped_length ), data.end() );
    }
    // 接着处理下一个可能有重合的元素
    rear_iter = unassembled_substrings_.lower_bound( next_rear );
  }
  // 更新 newindex
  if ( first_index > unassembled_index_ ) {
    auto front_iter = unassembled_substrings_.upper_bound( new_index );
    if ( front_iter != unassembled_substrings_.begin() ) {
      front_iter--;
      const auto& [front_index, front_data] = *front_iter;
      if ( front_index + front_data.size() - 1 >= first_index ) {
        uint64_t overlapped_length = 0;
        if ( front_index + front_data.size() <= first_index + data.size() ) {
          overlapped_length = front_index + front_data.size() - first_index;
        } else {
          overlapped_length = data.size();
        }
        if ( overlapped_length == front_data.size() ) {
          unassembled_bytes_ -= front_data.size();
          unassembled_substrings_.erase( front_index );
        } else {
          data.erase( data.begin(), data.begin() + static_cast<int64_t>( overlapped_length ) );

          new_index = first_index + overlapped_length;
        }
      }
    }
  }
  // 将新数据插入到 map 中
  if ( !data.empty() ) {
    unassembled_bytes_ += data.size();
    cout << new_index << " + " << data << '\n' ; 
    unassembled_substrings_.insert( make_pair( new_index, std::move( data ) ) );
  }
  // 输出给 Writer
  for ( auto iter = unassembled_substrings_.begin(); iter != unassembled_substrings_.end();) {
    auto& [sub_index, sub_data] = *iter;
    if ( sub_index == unassembled_index_ ) {
      const uint64_t prev_bytes_pushed = output.bytes_pushed();
      output.push( sub_data );
      
      const uint64_t bytes_pushed = output.bytes_pushed();
      if ( bytes_pushed != prev_bytes_pushed + sub_data.size() ) {

        const uint64_t pushed_length = bytes_pushed - prev_bytes_pushed;
        unassembled_index_ += pushed_length;
        unassembled_bytes_ -= pushed_length;
        unassembled_substrings_.insert( make_pair( unassembled_index_, sub_data.substr( pushed_length ) ) );

        unassembled_substrings_.erase( sub_index );
        break;
      }
      unassembled_index_ += sub_data.size();
      unassembled_bytes_ -= sub_data.size();
      unassembled_substrings_.erase( sub_index );
      iter = unassembled_substrings_.find( unassembled_index_ );
    } else {
      break;
    }
  }

  if ( is_closed() ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return unassembled_bytes_;
}