#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float RGB_WEIGHTS[2] = {0.299, 0.587, 0.114};

float get_pixel(image im, int x, int y, int c)
{
    x = (x < 0) ? 0 : (x >= im.w) ? im.w - 1
                                  : x;
    y = (y < 0) ? 0 : (y >= im.h) ? im.h - 1
                                  : y;

    return im.data[c * im.w * im.h + y * im.w + x];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if (x >= 0 && x < im.w && y >= 0 && y < im.h)
    {
        im.data[c * im.w * im.h + y * im.w + x] = v;
    }
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    copy.w = im.w;
    copy.h = im.h;
    copy.c = im.c;

    int num_elements = im.w * im.h * im.c;
    copy.data = (float *)(malloc(num_elements * sizeof(float)));

    if (copy.data != NULL)
    {
        for (int i = 0; i < num_elements; ++i)
        {
            copy.data[i] = im.data[i];
        }

        return copy;
    }
    else
    {
        fprintf(stderr, "Failed to allocate memory for image copy.\n");
    }
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);

    int num_pixels = gray.w * gray.h;
    gray.data = (float *)(malloc(num_pixels * sizeof(float)));

    if (gray.data != NULL)
    {

        for (int channel = 0; channel < im.c; ++channel)
        {
            for (int i = 0; i < num_pixels; ++i)
            {

                if (channel == 0)
                {
                    gray.data[i] = 0;
                }

                gray.data[i] += RGB_WEIGHTS[channel] * im.data[channel * im.h * im.w + i];
            }
        }
    }

    return gray;
}

void shift_image(image im, int c, float v)
{
    for (int i = 0; i < im.h * im.w; ++i)
    {
        im.data[c * im.h * im.w + i] = im.data[i] + v;
    }
    clamp_image(im);
}

void clamp_image(image im)
{
    for (int i = 0; i < im.c * im.h * im.w; ++i)
    {
        if (im.data[i] > 1)
        {
            im.data[i] = 1;
        }

        if (im.data[i] < 0)
        {
            im.data[i] = 0;
        }
    }
}

void rgb_to_hsv(image im)
{
    // TODO Fill this in
}

void hsv_to_rgb(image im)
{
    // TODO Fill this in
}
