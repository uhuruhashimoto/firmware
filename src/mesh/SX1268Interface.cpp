#include "SX1268Interface.h"
#include "configuration.h"
#include "error.h"

SX1268Interface::SX1268Interface(RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq, RADIOLIB_PIN_TYPE rst, RADIOLIB_PIN_TYPE busy,
                                 SPIClass &spi)
    : SX126xInterface(cs, irq, rst, busy, spi)
{
}

float SX1268Interface::getFreq()
{
    // Set frequency to default of EU_433 if outside of allowed range (e.g. when region is UNSET)
    if (savedFreq < 410 || savedFreq > 810)
        return 433.125f;
    else
        return savedFreq;
}