/*coding: utf-8*/
#include "Filter.h"

void Mean_Filter_Init(mean_filter_t *filter)
{
    filter->index = 0;
    filter->sum = 0;
    filter->flag = 0;
    filter->filter_out = 0;
}

void Mean_Filter_Update(mean_filter_t *filter, int16_t data)
{
    if (!filter->flag)
    {
        for (uint16_t i = 0; i < MEAN_FILTER_SIZE; i++)
        {
            filter->buffer[i] = data;
        }
        filter->sum = (int32_t)data * MEAN_FILTER_SIZE;
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

    /* 四舍五入定点除法 */
    filter->filter_out = (filter->sum + MEAN_FILTER_SIZE / 2) / MEAN_FILTER_SIZE;
}

/*
Usage example:
    // Initialize moving average filter
    mean_filter_t my_mean;
    Mean_Filter_Init(&my_mean);

    // Update with new samples
    for(int i = 0; i < 100; i++) {
        int16_t raw = get_adc_sample();
        Mean_Filter_Update(&my_mean, raw);
        int32_t smoothed = my_mean.filter_out;  // Get filtered raw ADC value
    }
*/
