#pragma once
#include <list>

class DropRateCalculator
{
public:
  DropRateCalculator();
  ~DropRateCalculator();

  void setPacketDistance(int packetDistance);
  void latestInput(int frameNumber);
  float getDropRate();
  void reset();

private:
  std::list<bool> drops;
  int latestFrame;
  int packetDistance;
};