#include <iostream>
#include "./DropRateCalculator.hpp"

DropRateCalculator::DropRateCalculator()
{
  latestFrame = -1;
  this->packetDistance = 1;
}

DropRateCalculator::~DropRateCalculator()
{
}

void DropRateCalculator::setPacketDistance(int packetDistance)
{
  this->packetDistance = packetDistance;
}

void DropRateCalculator::latestInput(int frameNumber)
{
  if (latestFrame == -1)
  {
    latestFrame = frameNumber;
    return;
  }

  if (frameNumber - latestFrame == this->packetDistance)
  {
    latestFrame = frameNumber;
    drops.push_back(false);
  }
  else if (frameNumber - latestFrame > this->packetDistance)
  {
    latestFrame = frameNumber;
    drops.push_back(true);
  }

  while (drops.size() > 100)
  {
    drops.pop_front();
  }
}

float DropRateCalculator::getDropRate()
{
  if (drops.size() == 0)
    return 0.0f;

  int dropsCount = 0;
  for (auto drop : drops)
  {
    if (drop)
      dropsCount++;
  }

  return (float)dropsCount / (float)drops.size();
}

void DropRateCalculator::reset()
{
  drops.clear();
  latestFrame = -1;
}