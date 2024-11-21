add_library(network_library 
  OBJECT
  ./util/Compression.cpp
  ./server/inputBuffer/DropRateCalculator.cpp
  ./server/inputBuffer/BufferSizeCalculator.cpp
  ./packets/input/IncomingNetworkPacket.cpp
  ./packets/output/OutgoingNetworkPacket.cpp
)