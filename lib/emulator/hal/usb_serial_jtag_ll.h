#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool usb_serial_jtag_ll_txfifo_writable(void) {
    return true;
}

#ifdef __cplusplus
}
#endif
