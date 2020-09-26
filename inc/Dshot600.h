#ifndef __DSHOT600_H__
#define __DSHOT600_H__

#include <stdint.h>
#include <DMAChannel.h>

#define DSHOT_MESSAGE_LEN         16 * 3
#define DSHOT_TRANSMISSION_LEN    ((DSHOT_MESSAGE_LEN * 11) / 10)  // 10% dead time

#define DSHOT_150_FREQ      3*149700.5988023952
#define DSHOT_TYPE          600

class Dshot600
{
    public:
        Dshot600() { }

        void begin(uint32_t _pinPos1, uint32_t _pinPos2, uint32_t _pinPos3, uint32_t _pinPos4);
        void setCommand(float commands[4], const int32_t telemetryMotor);
        void enable();
        void disable();

    public:
        uint8_t txBuffer0[DSHOT_TRANSMISSION_LEN];
        uint8_t txBuffer1[DSHOT_TRANSMISSION_LEN];
        uint32_t selectedIndex;

        DMAChannel dmaTx;

        uint32_t  pinPos[4];
};

#endif