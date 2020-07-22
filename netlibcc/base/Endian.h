#ifndef NETLIBCC_BASE_ENDIAN_H_
#define NETLIBCC_BASE_ENDIAN_H_

#include <endian.h>
#include <cstdint>

// man endian
// uint16_t htobe16(uint16_t host_16bits);
// uint16_t htole16(uint16_t host_16bits);
// uint16_t be16toh(uint16_t big_endian_16bits);
// uint16_t le16toh(uint16_t little_endian_16bits);

// uint32_t htobe32(uint32_t host_32bits);
// uint32_t htole32(uint32_t host_32bits);
// uint32_t be32toh(uint32_t big_endian_32bits);
// uint32_t le32toh(uint32_t little_endian_32bits);

// uint64_t htobe64(uint64_t host_64bits);
// uint64_t htole64(uint64_t host_64bits);
// uint64_t be64toh(uint64_t big_endian_64bits);
// uint64_t le64toh(uint64_t little_endian_64bits);

namespace netlibcc {
namespace net {
namespace sockets {

// wrapper functions of endian conversion that included in <endian.h>

// network to host

inline uint16_t netToHost16(uint16_t net16) {
    return be16toh(net16);
}

inline uint32_t netToHost32(uint32_t net32) {
    return be32toh(net32);
}

inline uint64_t netToHost64(uint64_t net64) {
    return be64toh(net64);
}

// host to netowrk

inline uint16_t hostToNet16(uint16_t host16) {
    return htobe16(host16);
}

inline uint32_t hostToNet32(uint32_t host32) {
    return htobe32(host32);
}

inline uint64_t hostToNet64(uint64_t host64) {
    return htobe64(host64);
}

} // namespace sockets
} // namespace net
} // namespace netlibcc

#endif // NETLIBCC_BASE_ENDIAN_H_