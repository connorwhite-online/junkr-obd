#pragma once
#include <stdint.h>

namespace can_proto {

constexpr uint32_t ID_EGT     = 0x100;  // int16 °F, big-endian
constexpr uint32_t ID_COOLANT = 0x101;  // int16 °F, big-endian
constexpr uint32_t ID_MAP_PSI = 0x102;  // int16 psi × 10, big-endian

constexpr uint32_t CAN_BITRATE_BPS = 500000;
constexpr uint32_t TX_PERIOD_MS = 50;   // 20 Hz publish cadence

constexpr int16_t SENTINEL_INVALID = INT16_MIN;

}  // namespace can_proto
