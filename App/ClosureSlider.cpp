#include "ClosureSlider.h"
#include "main.h"

#define ADC_POTS ADC1

void ClosureSlider::init() {

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_0, LL_ADC_DMA_GetRegAddr(ADC_POTS, LL_ADC_DMA_REG_REGULAR_DATA));
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)&_value);

    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, 1);

    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);

    LL_ADC_Enable(ADC_POTS);
    LL_ADC_REG_StartConversionSWStart(ADC_POTS);
}