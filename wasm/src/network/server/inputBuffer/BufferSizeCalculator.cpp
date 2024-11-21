#include "./BufferSizeCalculator.hpp"
#include <iostream>

const float MAX_BUFFER_SIZE = 10.0f;
const float MIN_BUFFER_SIZE = 5.0f;
const float DEFAULT_BUFFER_SIZE = 7.0f;

// Thresholds to adjust buffer size
const float LOW_DROP_RATE_THRESHOLD = 0.00f;  // 5% drop rate
const float HIGH_DROP_RATE_THRESHOLD = 0.40f; // 20% drop rate

// Increment or decrement values for buffer size
const float BUFFER_INCREMENT = 0.1f;
const float BUFFER_DECREMENT = 0.05f;

BufferSizeCalculator::BufferSizeCalculator(DropRateCalculator *dropRateCalculator)
{
  this->dropRateCalculator = dropRateCalculator;
  this->bufferSize = DEFAULT_BUFFER_SIZE;
}

BufferSizeCalculator::~BufferSizeCalculator()
{
}

int BufferSizeCalculator::getBufferSize()
{
  float dropRate = this->dropRateCalculator->getDropRate();
  float bufferRangePercentage = std::max(std::min((dropRate - LOW_DROP_RATE_THRESHOLD) / HIGH_DROP_RATE_THRESHOLD, 1.0f), 0.0f);
  float targetBufferSize = bufferRangePercentage * (MAX_BUFFER_SIZE - MIN_BUFFER_SIZE) + MIN_BUFFER_SIZE;

  if (targetBufferSize > this->bufferSize)
    this->bufferSize += BUFFER_INCREMENT;
  else if (targetBufferSize < this->bufferSize)
    this->bufferSize -= BUFFER_DECREMENT;

  if (this->bufferSize < MIN_BUFFER_SIZE)
    this->bufferSize = MIN_BUFFER_SIZE;
  else if (this->bufferSize > MAX_BUFFER_SIZE)
    this->bufferSize = MAX_BUFFER_SIZE;

  return static_cast<int>(this->bufferSize);
}
