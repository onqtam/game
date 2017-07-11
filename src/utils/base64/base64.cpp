#include "base64.h"

namespace base64
{
namespace
{
    uint8* createEncodeTable(const char b62, const char b63) {
        static uint8 table[64];
        static int   initialized = false;
        if(!initialized) {
            for(uint8 i = 0; i < 26; i++) {
                table[i]      = 'A' + i;
                table[26 + i] = 'a' + i;
            }
            for(uint8 i       = 0; i < 10; i++)
                table[52 + i] = '0' + i;
            initialized       = true;
        }
        table[62] = b62;
        table[63] = b63;
        return table;
    }

    uint8* createDecodeTable(const char b62, const char b63) {
        static uint8 table[256];
        static int   initialized = false;
        if(!initialized) {
            memset(table, 0, 256);
            for(uint8 i  = 'A'; i <= 'Z'; i++)
                table[i] = i - 'A';
            for(uint8 i  = 'a'; i <= 'z'; i++)
                table[i] = 26 + i - 'a';
            for(uint8 i  = '0'; i <= '9'; i++)
                table[i] = 52 + i - '0';
            initialized  = true;
        }
        table[b62] = 62;
        table[b63] = 63;
        return table;
    }
} //anonymous namespace

int getEncodedBufferMaxSize(const int binaryDataSize) {
    int byteTriplets = binaryDataSize / 3;
    if(binaryDataSize % 3) {
        byteTriplets += 1;
    }
    return (byteTriplets * 4);
}

int getDecodedBufferMaxSize(const int encodedDataSize) {
    int byteQuadruplets = encodedDataSize / 4;
    return (byteQuadruplets * 3);
}

void encode(const uint8* data, const int dataLen, uint8* encoded, const int encodedBuffSize,
            const char b62, const char b63, const char pad) {
    hassert(data);
    hassert(dataLen > 0);
    uint8* table         = createEncodeTable(b62, b63);
    int    padding       = (dataLen % 3) ? (3 - dataLen % 3) : 0;
    int    encodedLength = 4 * (dataLen / 3) + (padding ? 4 : 0);
    hassert(encodedBuffSize >= encodedLength);
    int block;
    for(block = 0; block < dataLen / 3; block++) {
        uint32 bytes = (uint32)data[block * 3] << 16;
        bytes |= (uint32)data[block * 3 + 1] << 8;
        bytes |= (uint32)data[block * 3 + 2];
        int offset          = block * 4;
        encoded[offset]     = table[(bytes >> 18) & 0x3f];
        encoded[offset + 1] = table[(bytes >> 12) & 0x3f];
        encoded[offset + 2] = table[(bytes >> 6) & 0x3f];
        encoded[offset + 3] = table[bytes & 0x3f];
    }
    if(padding) {
        uint32 bytes = (uint32)data[block * 3] << 16;
        if(padding == 1)
            bytes |= (uint32)data[1 + block * 3] << 8;
        int offset          = block * 4;
        encoded[offset]     = table[(bytes >> 18) & 0x3f];
        encoded[offset + 1] = table[(bytes >> 12) & 0x3f];
        encoded[offset + 2] = table[(bytes >> 6) & 0x3f];
        encoded[offset + 3] = pad;
        if(padding == 2)
            encoded[offset + 2] = pad;
    }
}

int decode(const uint8* data, const int dataLen, uint8* decoded, const int decodedBuffSize,
           const char b62, const char b63, const char pad) {
    hassert(data);
    hassert(dataLen % 4 == 0);
    uint8*    table                   = createDecodeTable(b62, b63);
    const int requiredDecodedBuffSize = getDecodedBufferMaxSize(dataLen);
    hassert(decodedBuffSize >= requiredDecodedBuffSize);

    for(int block = 0; block < dataLen / 4; block++) {
        const uint8* chars  = ((const uint8*)data + block * 4);
        int          offset = block * 3;
        decoded[offset]     = table[chars[0]] << 2 | table[chars[1]] >> 4;
        decoded[offset + 1] = table[chars[1]] << 4 | table[chars[2]] >> 2;
        decoded[offset + 2] = table[chars[2]] << 6 | table[chars[3]];
    }

    int          countOfDecodedBytes = (dataLen / 4) * 3;
    const uint8* padchar             = (const uint8*)data + dataLen - 1;
    while(countOfDecodedBytes && *padchar-- == pad)
        --countOfDecodedBytes; // padding is not part of the data
    return countOfDecodedBytes;
}

int decodeInplace(uint8* data, const int dataLen) { return decode(data, dataLen, data, dataLen); }

} // namespace base64
