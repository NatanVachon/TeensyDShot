#include "Dshot600.h"
#include "Arduino.h"

#define DSHOT_ZERO        {1, 0, 0}
#define DSHOT_ONE         {1, 1, 0}


void Dshot600::begin(uint32_t _pinPos1, uint32_t _pinPos2, uint32_t _pinPos3, uint32_t _pinPos4)
{
    uint32_t i;
    uint32_t bit[3] = DSHOT_ZERO;    

    // Initialize class variables
    pinPos[0] = _pinPos1;
    pinPos[1] = _pinPos2;
    pinPos[2] = _pinPos3;
    pinPos[3] = _pinPos4;

    // Initialize index
    selectedIndex = 1;

    for (i = 0; i < DSHOT_MESSAGE_LEN; i += 3)
    {
        txBuffer0[i]     = (bit[0]<<pinPos[0]) | (bit[0]<<pinPos[1]) | (bit[0]<<pinPos[2]) | (bit[0]<<pinPos[3]);
        txBuffer0[i + 1] = (bit[1]<<pinPos[0]) | (bit[1]<<pinPos[1]) | (bit[1]<<pinPos[2]) | (bit[1]<<pinPos[3]);
        txBuffer0[i + 2] = (bit[2]<<pinPos[0]) | (bit[2]<<pinPos[1]) | (bit[2]<<pinPos[2]) | (bit[2]<<pinPos[3]);
        txBuffer1[i]     = (bit[0]<<pinPos[0]) | (bit[0]<<pinPos[1]) | (bit[0]<<pinPos[2]) | (bit[0]<<pinPos[3]);
        txBuffer1[i + 1] = (bit[1]<<pinPos[0]) | (bit[1]<<pinPos[1]) | (bit[1]<<pinPos[2]) | (bit[1]<<pinPos[3]);
        txBuffer1[i + 2] = (bit[2]<<pinPos[0]) | (bit[2]<<pinPos[1]) | (bit[2]<<pinPos[2]) | (bit[2]<<pinPos[3]);
    }

    // Initialize DMA channel
    dmaTx.disable();

    dmaTx.sourceBuffer((uint8_t*)txBuffer0, DSHOT_TRANSMISSION_LEN);
    dmaTx.destination(GPIOD_PDOR);

    dmaTx.transferSize(1);
    dmaTx.transferCount(DSHOT_TRANSMISSION_LEN);

    dmaTx.triggerAtHardwareEvent(DMAMUX_SOURCE_FTM0_CH0);


    // Initialize FTM counter for DMA trigger
    FTM0_SC = 0;

    analogWriteFrequency(22, DSHOT_TYPE / 150 * DSHOT_150_FREQ); // FTM0 channel 0

    FTM0_OUTMASK = 0xFF;       // Use mask to disable outputs
    FTM0_CNTIN = 0;            // Counter initial value
    FTM0_COMBINE = 0x00003333; // COMBINE=1, COMP=1, DTEN=1, SYNCEN=1
    FTM0_MODE = 0x01;          // Enable FTM0
    FTM0_SYNC = 0x02;          // PWM sync @ max loading point enable

    FTM0_C0V = FTM0_MOD;

    FTM0_SYNC |= 0x80;         // set PWM value update
    FTM0_C0SC = 0x28 | (1<<6) | (1<<0); // PWM output, edge aligned, positive signal

    FTM0_CNT = 0;

    FTM0_CONF |= FTM_CONF_GTBEOUT; // GTBEOUT 1
    
    dmaTx.enable();
}

// Telemetry is number of the motor, if no motor is 0 TODO ca marche pas ca
void Dshot600::setCommand(float commands[4], const int32_t telemetryMotor)
{
    uint32_t i, j;
    uint8_t* txBuffer;
    uint32_t packets[4] = { 0 };

    // Compute digital commands
    for (i = 0; i < 4; i++)
    {
        packets[i] = (uint32_t)(20.0f * commands[i] + (commands[i] > 0.0f ? 47 : 0)) << 1; // Converts from 0.0 -> 100.0 to 47 -> 2047
    }

    // Update telemetry data
    if (telemetryMotor >= 0)
        packets[telemetryMotor] |= 1;

    // Update CRC
    // Compute CRC
    uint32_t csum;
    uint32_t csum_data;
    for (i = 0; i < 4; i++)
    {
        csum = 0;
        csum_data = packets[i];
        for (j = 0; j < 3; j++)
        {
            csum ^= csum_data;   // xor data
            csum_data >>= 4;
        }

        csum &= 0xF;
        packets[i] = (packets[i] << 4) | csum;
    }

    // Update Tx Buffer
    selectedIndex ^= 1;
    txBuffer = selectedIndex == 0 ? txBuffer0 : txBuffer1;

    for (i = 0; i < 16; i++)
    {
        txBuffer[3 * i + 1] =   (((packets[0] >> (15 - i)) & 1) << pinPos[0]) |
                                (((packets[1] >> (15 - i)) & 1) << pinPos[1]) | 
                                (((packets[2] >> (15 - i)) & 1) << pinPos[2]) |
                                (((packets[3] >> (15 - i)) & 1) << pinPos[3]);
    }

    // Wait for DMA to finish
    while (!dmaTx.complete());

    // Switch source buffer
    dmaTx.TCD->SADDR = txBuffer;
}