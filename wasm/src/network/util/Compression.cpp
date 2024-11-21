#include "./Compression.hpp"
#include <bzlib.h>

int compress(const char *data, int length, char *compressed)
{
  bz_stream stream;

  for (int i = 0; i < 4; i++)
  {
    compressed[i] = data[i];
  }

  stream.next_in = (char*)(data + 4);
  stream.next_out = compressed + 4;
  stream.avail_in = length - 4;
  stream.avail_out = length - 4;

  stream.bzalloc = NULL;
  stream.bzfree = NULL;
  stream.opaque = NULL;

  BZ2_bzCompressInit(&stream, 9, 0, 30);

  int ret;
  do
  {
    ret = BZ2_bzCompress(&stream, BZ_FINISH);
  } while (ret == BZ_STREAM_END);

  int byteSizeOut = stream.total_out_lo32;

  BZ2_bzCompressEnd(&stream);

  return byteSizeOut + 4;
}

int decompress(char *data, int length, char *decompressed, int decompressedLength)
{
  bz_stream stream;

  for (int i = 0; i < 4; i++)
  {
    decompressed[i] = data[i];
  }

  stream.next_in = data + 4;
  stream.next_out = decompressed + 4;
  stream.avail_in = length - 4;
  stream.avail_out = decompressedLength;

  stream.bzalloc = NULL;
  stream.bzfree = NULL;
  stream.opaque = NULL;

  BZ2_bzDecompressInit(&stream, 0, 0);
  int ret;
  do
  {
    ret = BZ2_bzDecompress(&stream);
  } while (ret == BZ_STREAM_END);

  int byteSizeOut = stream.total_out_lo32;

  BZ2_bzDecompressEnd(&stream);

  return byteSizeOut + 4;
}