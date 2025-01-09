#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im)
{

    int channel_pixel_sums[im.c];

    for (int i = 0; i < im.c; ++i)
    {
        channel_pixel_sums[i] = 0;
    }

    for (int i = 0; i < im.w; ++i)
    {
        for (int j = 0; j < im.h; ++j)
        {
            for (int ch = 0; ch < im.c; ++ch)
            {

                channel_pixel_sums[ch] += get_pixel(im, i, j, ch);

                if (i == im.w - 1 && j == im.h - 1 && channel_pixel_sums[ch] == 0)
                {
                    channel_pixel_sums[ch] = 1;
                }
            }
        }
    }

    for (int i = 0; i < im.w; ++i)
    {
        for (int j = 0; j < im.h; ++j)
        {
            for (int ch = 0; ch < im.c; ++ch)
            {
                set_pixel(im, i, j, ch, get_pixel(im, i, j, ch) / channel_pixel_sums[ch]);
            }
        }
    }
}

image make_box_filter(int w)
{
    image bf = make_image(w, w, 1);

    for (int i = 0; i < bf.w; ++i)
    {
        for (int j = 0; j < bf.h; ++j)
        {
            set_pixel(bf, i, j, 0, 1);
        }
    }
    l1_normalize(bf);
    return bf;
}

image convolve_image(image im, image filter, int preserve)
{
    assert(filter.c == 1 || filter.c == im.c);
}

image make_highpass_filter()
{
    // TODO
    return make_image(1, 1, 1);
}

image make_sharpen_filter()
{
    // TODO
    return make_image(1, 1, 1);
}

image make_emboss_filter()
{
    // TODO
    return make_image(1, 1, 1);
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO

image make_gaussian_filter(float sigma)
{
    // TODO
    return make_image(1, 1, 1);
}

image add_image(image a, image b)
{
    // TODO
    return make_image(1, 1, 1);
}

image sub_image(image a, image b)
{
    // TODO
    return make_image(1, 1, 1);
}

image make_gx_filter()
{
    // TODO
    return make_image(1, 1, 1);
}

image make_gy_filter()
{
    // TODO
    return make_image(1, 1, 1);
}

void feature_normalize(image im)
{
    // TODO
}

image *sobel_image(image im)
{
    // TODO
    return calloc(2, sizeof(image));
}

image colorize_sobel(image im)
{
    // TODO
    return make_image(1, 1, 1);
}
