#pragma once
#include <list>
#include "./DropRateCalculator.hpp"

class BufferSizeCalculator
{
public:
  BufferSizeCalculator(DropRateCalculator *dropRateCalculator);
  ~BufferSizeCalculator();

  int getBufferSize();

private:
  DropRateCalculator *dropRateCalculator;
  float bufferSize;
};