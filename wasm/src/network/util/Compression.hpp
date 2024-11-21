#pragma once

int compress(const char *data, int length, char *compressed);
int decompress(char *data, int length, char *decompressed, int decompressedLength);