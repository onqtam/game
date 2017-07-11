#pragma once

namespace base64
{
/// Helper function that calculates encoded buffer size based on the size of input binary data.
/// @return Returned decoded buffer size is large enough to hold encoded string plus padding characters if required.
HAPI int getEncodedBufferMaxSize(int binaryDataSize);

/// Helper function that calculates decoded data size based on the size of encoded buffer.
/// @return Returned decoded buffer size is large enough to hold decoded data plus padding characters if there are any.
HAPI int getDecodedBufferMaxSize(int encodedDataSize);

/// Encodes binary data into ASCII string. Binary bytes are iterated in chunks of 3 which are converted into 4 bytes of characters.
/// The encoding CAN'T be done inplace direclty on the binary buffer, because 3 bytes are written into 4 bytes and binary data will be corrupted.
/// Use separate buffer that will hold the ASCII string.
/// Note that the function will not write a closing NULL character to the resulting string.
/// Note that this function is NOT thread-safe, if anyone uses it with values for 'b62' and 'b63' that are not the defaults.
/// Also it should be considered NOT thread-safe even when called for the first time.
/// @param data - Input data to encode.
/// @param dataLen - Length of input data to be encoded in bytes.
/// @param encoded - Pointer to the buffer that will receive encoded binary data. 'data' and 'encoded' MUST NOT point to the same memory.
/// @param encodedBuffSize - Size in bytes of the buffer where encoded data will be written. This must be at least getEncodedBufferMaxSize(dataLen).
/// @param b62 - Optional character to override the default value for the 62-th byte in encoding table.
/// @param b63 - Optional character to override the default value for the 63-th byte in encoding table.
/// @param pad - Optional character to override the default value for the padding byte.
HAPI void encode(const uint8* data, int dataLen, uint8* encoded, int encodedBuffSize,
                 char b62 = '+', char b63 = '/', char pad = '=');

/// Decodes ASCII string back to binary data. ASCII characters are iterated in chunks of 4 which are converted into 3 bytes of binary data.
/// The decoding CAN be done inplace directly on the ASCII string because 4 bytes are written into 3 bytes and string data will not be corrupted.
/// Note that this function is NOT thread-safe, if anyone uses it with values for 'b62' and 'b63' that are not the defaults.
/// Also it should be considered NOT thread-safe even when called for the first time.
/// @param data - Input encoded data. Size of input data must be multiple of 4.
/// @param dataLen - Length of input data to be decoded in bytes.
/// @param decoded - Pointer to the buffer that will receive decoded binary data. Both data and decoded can point to same memory.
/// @param decodedBuffSize - Size in bytes of the buffer where decoded data will be written. If case decoded buffer is different drom data buffer,
///        the decoded buffer must be large enough to hold decoded binary data.
/// @param b62 - Optional character to override the default value for the 62-th byte in decoding table.
/// @param b63 - Optional character to override the default value for the 63-th byte in decoding table.
/// @param pad - Optional character to override the default value for the padding byte.
/// @return Returns the number of decoded bytes, or -1 in case of error.
HAPI int decode(const uint8* data, int dataLen, uint8* decoded, int decodedBuffSize, char b62 = '+',
                char b63 = '/', char pad = '=');

/// Helper function for inplace decoding of ASCII string to binary data.
/// @param data - Pointer to the buffer holding the ascii string data. Note that decoded data will be written over the encoded data.
/// @return Returns the number of decoded bytes, or -1 in case of error.
HAPI int decodeInplace(uint8* data, int dataLen);

} // namespace base64
