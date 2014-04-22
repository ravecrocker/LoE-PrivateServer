#ifndef SENDMESSAGE_H
#define SENDMESSAGE_H

#include "message.h"

// Resend the udp message if we didn't get an ACK before this timeouts
#define UDP_RESEND_TIMEOUT 500
// If we send multiple reliable messages before this timeouts, group them before sending. Increases the latency.
#define UDP_GROUPING_TIMEOUT 25

#endif // SENDMESSAGE_H
