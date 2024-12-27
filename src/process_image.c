#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"
#include <stdlib.h>

float RGB_WEIGHTS[3] = {0.299, 0.587, 0.114};

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
    }
    else
    {
        fprintf(stderr, "Failed to allocate memory for image copy.\n");
    }

    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);

    int num_pixels = gray.w * gray.h;
    gray.data = (float *)(malloc(num_pixels * sizeof(float)));

    if (gray.data != NULL)
    {

        for (int y = 0; y < gray.h; ++y)
        {
            for (int x = 0; x < gray.w; ++x)
            {
                float gray_val = 0;

                for (int c = 0; c < im.c; ++c)
                {
                    gray_val += RGB_WEIGHTS[c] * get_pixel(im, x, y, c);
                }
                set_pixel(gray, x, y, 0, gray_val);
            }
        }
    }

    return gray;
}

void shift_image(image im, int c, float v)
{
    for (int i = 0; i < im.h * im.w; ++i)
    {
        im.data[c * im.h * im.w + i] = im.data[c * im.h * im.w + i] + v;
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

float three_way_max(float a, float b, float c)
{
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

void rgb_to_hsv(image im)
{

    if (im.c != 3)
    {
        return;
    }

    int num_pixels = im.w * im.h;

    float *r_channel_start = im.data;
    float *g_channel_start = im.data + num_pixels;
    float *b_channel_start = im.data + 2 * num_pixels;

    for (int i = 0; i < num_pixels; ++i)
    {

        float value = three_way_max(r_channel_start[i], g_channel_start[i], b_channel_start[i]);
        float min = three_way_min(r_channel_start[i], g_channel_start[i], b_channel_start[i]);
        float C = value - min;
        float sat = 0;

        if (value != 0)
        {
            sat = C / value;
        }

        float hue = 0;

        if (C > 0)
        {
            if (value == r_channel_start[i])
            {
                hue = (g_channel_start[i] - b_channel_start[i]) / C + (g_channel_start[i] < b_channel_start[i] ? 6 : 0);
            }

            else if (value == g_channel_start[i])
            {
                hue = (b_channel_start[i] - r_channel_start[i]) / C + 2;
            }

            else
            {
                hue = (r_channel_start[i] - g_channel_start[i]) / C + 4;
            }
            hue /= 6;
        }

        r_channel_start[i] = hue;
        g_channel_start[i] = sat;
        b_channel_start[i] = value;
    }
}

void hsv_to_rgb(image im)
{
    if (im.c != 3)
    {
        return;
    }

    int num_pixels = im.w * im.h;

    float *h_channel_start = im.data;
    float *s_channel_start = im.data + num_pixels;
    float *v_channel_start = im.data + 2 * num_pixels;

    for (int i = 0; i < num_pixels; ++i)
    {
        float h = h_channel_start[i];
        float s = s_channel_start[i];
        float v = v_channel_start[i];

        float r, g, b;

        if (s == 0)
        {

            r = g = b = v;
        }
        else
        {

            h *= 6;
            int sector = (int)h;
            float fractional = h - sector;

            float p = v * (1 - s);
            float q = v * (1 - s * fractional);
            float t = v * (1 - s * (1 - fractional));

            switch (sector % 6)
            {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            default:
                r = v;
                g = p;
                b = q;
                break;
            }
        }

        h_channel_start[i] = r;
        s_channel_start[i] = g;
        v_channel_start[i] = b;
    }
}
