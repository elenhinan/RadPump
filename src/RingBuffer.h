#pragma once
#include <arduino.h>

template <class datatype, int length>
class RingBuffer
{
private:
    uint8_t volatile read_index;
    uint8_t write_index;
    uint8_t const index_mask = ((length * sizeof(datatype))-1))
    datatype buffer[length];
public:
    RingBuffer();
    inline datatype peak() { return buffer[read_index]; }
    inline datatype pull() { datatype value = buffer[read_index]; read_index = (write_index+1)&index_mask; return value; }
    inline void push(datatype value) { buffer[write_index] = value; movebuffer_w = (movebuffer_w+1)&index_mask; }
    inline bool isempty() { return read_index == write_index; }
    inline bool isfull() { return read_index == ((write_index+1)&index_mask; }
};