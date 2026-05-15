/*coding: utf-8*/
#include "Filter.h"

void IIR_Filter_Init(iir_filter_t *iir, float alpha)
{
    iir->filter_out = 0;
    iir->alpha = alpha;
    iir->flag = 0;
}

void IIR_Filter_Update(iir_filter_t *iir, uint16_t data)
{
    if (!iir->flag)
    {
        iir->filter_out = data;
        iir->flag = 1;
    }
    iir->filter_out = (1 - iir->alpha) * data + iir->alpha * iir->filter_out;
}

void Mean_Filter_Init(mean_filter_t *filter)
{
    filter->index = 0;
    filter->sum = 0;
    filter->flag = 0;
    filter->filter_out = 0;
}

void Mean_Filter_Update(mean_filter_t *filter, float data)
{
    if (!filter->flag)
    {
        for (uint16_t i = 0; i < MEAN_FILTER_SIZE; i++)
        {
            filter->buffer[i] = data;
        }
        filter->sum = data * MEAN_FILTER_SIZE;
        filter->filter_out = data;
        filter->flag = 1;
    }

    filter->sum -= filter->buffer[filter->index];
    filter->buffer[filter->index] = data;
    filter->sum += data;

    filter->index++;
    if (filter->index >= MEAN_FILTER_SIZE)
    {
        filter->index = 0;
    }

    filter->filter_out = filter->sum / MEAN_FILTER_SIZE;
}

/*
Usage example:
    // Initialize IIR filter with alpha=0.9 (high smoothing)
    iir_filter_t my_iir;
    IIR_Filter_Init(&my_iir, 0.9f);

    // Update with new samples
    for(int i = 0; i < 100; i++) {
        uint16_t raw_data = get_adc_sample();
        IIR_Filter_Update(&my_iir, raw_data);
        float filtered = my_iir.filter_out;  // Get filtered value
    }

    // Initialize moving average filter
    mean_filter_t my_mean;
    Mean_Filter_Init(&my_mean);

    // Update with new samples
    for(int i = 0; i < 100; i++) {
        float data = (float)get_adc_sample();
        Mean_Filter_Update(&my_mean, data);
        float smoothed = my_mean.filter_out;  // Get smoothed value
    }
*/
