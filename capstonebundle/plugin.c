#include "arm/arm32/arm32.h"
#include "arm/arm32/thumb.h"
#include "arm/arm64.h"
#include "mos65xx/mos6502.h"
#include <redasm/redasm.h>

void rd_plugin_create(void) {
    rd_register_processor(&THUMB_LE);
    rd_register_processor(&THUMB_BE);

    rd_register_processor(&ARM32_LE);
    rd_register_processor(&ARM32_BE);

    rd_register_processor(&ARM64_LE);
    rd_register_processor(&ARM64_BE);

    rd_register_processor(&MOS6502);
}

const char* rd_plugin_version(void) {
    int major, minor;
    cs_version(&major, &minor);
    return rd_format("1.0-capstone-%d.%d", major, minor);
}
