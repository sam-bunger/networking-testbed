#pragma once

#include <memory>

class IncomingNetworkPacket
{
public:
	IncomingNetworkPacket(const void *p, int size);

	bool isValid() const;

protected:
	const char*getPacket() const;
	int getPacketSize() const;
	void notValid();


private:
	std::shared_ptr<const char[]> packetData;
	int size;
	bool isPacketValid;
};