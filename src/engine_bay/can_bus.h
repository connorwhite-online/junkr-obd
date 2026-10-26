#pragma once
#include <stdint.h>
#include "can_protocol.h"

bool can_setup();
bool can_send(const can_proto::Frame& f);

// Drains the RX queue. Returns true and sets rpm when a tap 0x110 frame
// arrived since the last call.
bool can_poll_rpm(uint16_t& rpm);
