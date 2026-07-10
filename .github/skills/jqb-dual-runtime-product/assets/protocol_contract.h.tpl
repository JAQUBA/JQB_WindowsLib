#pragma once

// Shared protocol constants for firmware + desktop app.
// Keep this file language-compatible (C/C++).

#define PROTOCOL_VERSION            0x01
#define REPORT_ID_INFO              0x08
#define REPORT_ID_CONFIG            0x02

#define INFO_REPORT_SIZE            4
#define CONFIG_REPORT_SIZE          12

#define CAP_LONG_PRESS              0x01
#define CAP_MEDIA_KEYS              0x02

// INFO payload offsets
#define INFO_OFS_PROTOCOL_VERSION   0
#define INFO_OFS_KEY_COUNT          1
#define INFO_OFS_ENCODER_COUNT      2
#define INFO_OFS_CAPABILITIES       3
