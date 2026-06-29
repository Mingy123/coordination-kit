#pragma once

#include "sdkconfig.h"

#if CONFIG_CAMERA_MODULE_DFR1154
#define CAMERA_MODULE_NAME "DFR1154"
#define CAMERA_MODULE_SOC  "esp32s3"

/* Verified from DFRobot DFR1154 Arduino examples */
#define CAMERA_PIN_PWDN   -1
#define CAMERA_PIN_RESET  -1
#define CAMERA_PIN_XCLK   5
#define CAMERA_PIN_SIOD   8   /* SCCB SDA */
#define CAMERA_PIN_SIOC   9   /* SCCB SCL */

#define CAMERA_PIN_D7     4   /* Y9  */
#define CAMERA_PIN_D6     6   /* Y8  */
#define CAMERA_PIN_D5     7   /* Y7  */
#define CAMERA_PIN_D4     14  /* Y6  */
#define CAMERA_PIN_D3     17  /* Y5  */
#define CAMERA_PIN_D2     21  /* Y4  */
#define CAMERA_PIN_D1     18  /* Y3  */
#define CAMERA_PIN_D0     16  /* Y2  — XTAL_32K_N module pin → GPIO16 */

#define CAMERA_PIN_VSYNC  1
#define CAMERA_PIN_HREF   2
#define CAMERA_PIN_PCLK   15  /* XTAL_32K_P module pin → GPIO15 */
#endif
