#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

float gaussian(float x, float y, float sigma)
{
    return (1 / (TWOPI * sigma * sigma)) * exp(-((x * x + y * y) / (2 * (sigma * sigma))));
}

void l1_normalize(image im)
{

    int channel_pixel_sums[im.c];

    for (int ch = 0; ch < im.c; ++ch)
    {
        channel_pixel_sums[ch] = 0;

        for (int i = 0; i < im.w; ++i)
        {
            for (int j = 0; j < im.h; ++j)
            {
                channel_pixel_sums[ch] += get_pixel(im, i, j, ch);

                if (i == im.w - 1 && j == im.h - 1 && channel_pixel_sums[ch] == 0)
                {
                    channel_pixel_sums[ch] = 1;
                }
            }
        }

        for (int i = 0; i < im.w; ++i)
        {
            for (int j = 0; j < im.h; ++j)
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

float give_padding_if_need(image im, int x, int y, int c)
{
    if (x < 0 || y < 0)
    {
        return 0.;
    }
    else if (x > im.w || y > im.h)
    {
        return 0.;
    }
    else
    {
        return get_pixel(im, x, y, c);
    }
}

image convolve_image(image im, image filter, int preserve)
{
    assert(filter.c == 1 || filter.c == im.c);

    image final = preserve ? make_image(im.w, im.h, im.c) : make_image(im.w, im.h, 1);

    for (int ch = 0; ch < im.c; ++ch)
    {
        for (int i = 0; i < im.w; ++i)
        {

            for (int j = 0; j < im.h; ++j)
            {

                float conv_sum = 0; // each pixel i, j is mapped to a sum

                for (int fx = 0; fx < filter.w; ++fx)
                {
                    int x = i + fx - filter.w / 2;
                    for (int fy = 0; fy < filter.h; ++fy)
                    {
                        int y = j + fy - filter.h / 2;

                        float ipixel = get_pixel(im, x, y, ch);
                        float fpixel = get_pixel(filter, fx, fy, filter.c == 1 ? 0 : ch);
                        conv_sum += ipixel * fpixel;
                    }
                }

                preserve ? set_pixel(final, i, j, ch, conv_sum) : set_pixel(final, i, j, 0, get_pixel(final, i, j, 0) + conv_sum);
            }
        }
    }

    return final;
}

image make_highpass_filter()
{
    image hpf = make_image(3, 3, 1);

    float hpf_values[] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    for (int i = 0; i < 9; i++)
    {
        hpf.data[i] = hpf_values[i];
    }
    return hpf;
}

image make_sharpen_filter()
{
    image sf = make_image(3, 3, 1);

    float sf_values[] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    for (int i = 0; i < 9; ++i)
    {
        sf.data[i] = sf_values[i];
    }
    return sf;
}

image make_emboss_filter()
{
    image ef = make_image(3, 3, 1);

    float sf_values[] = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
    for (int i = 0; i < 9; ++i)
    {
        ef.data[i] = sf_values[i];
    }
    return ef;
}

image make_gaussian_filter(float sigma)
{
    int filter_size = 2 * (int)ceil(3 * sigma) + 1;

    image gf = make_image(filter_size, filter_size, 1);

    for (int i = 0; i < gf.w; ++i)
    {
        int x = i - filter_size / 2; // this is for calculating relative weights of each element based on how close they are to center
        for (int j = 0; j < gf.h; ++j)
        {
            int y = j - filter_size / 2;
            set_pixel(gf, i, j, 0, gaussian(x, y, sigma));
        }
    }
    l1_normalize(gf);
    return gf;
}

image add_image(image a, image b)
{
    assert(a.h == b.h && a.w == b.w && a.c == b.c);
    float sumpix;
    image sum = make_image(a.w, a.h, a.c);

    for (int i = 0; i < sum.h * sum.w * sum.c; ++i)
    {
        sumpix = a.data[i] + b.data[i];
        sum.data[i] = sumpix <= 1 ? sumpix : 1;
    }

    return sum;
}

image sub_image(image a, image b)
{
    assert(a.h == b.h && a.w == b.w && a.c == b.c);
    float diffpix;
    image diff = make_image(a.w, a.h, a.c);

    for (int i = 0; i < diff.h * diff.w * diff.c; ++i)
    {
        diffpix = a.data[i] - b.data[i];
        // diff.data[i] = diffpix >= 0 ? diffpix : 0; no good
        diff.data[i] = diffpix;
    }

    return diff;
}

image make_gx_filter()
{
    image gx = make_image(3, 3, 1);

    float gx_values[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    for (int i = 0; i < 9; ++i)
    {
        gx.data[i] = gx_values[i];
    }
    return gx;
}

image make_gy_filter()
{
    image gy = make_image(3, 3, 1);

    float gy_values[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    for (int i = 0; i < 9; ++i)
    {
        gy.data[i] = gy_values[i];
    }
    return gy;
}

void feature_normalize(image im)
{
    float min_val[im.c], max_val[im.c], temp, range;

    for (int ch = 0; ch < im.c; ++ch)
    {
        min_val[ch] = 1;
        max_val[ch] = 0;

        for (int i = 0; i < im.w; ++i)
        {
            for (int j = 0; j < im.h; ++j)
            {
                temp = get_pixel(im, i, j, ch);

                max_val[ch] = temp > max_val[ch] ? temp : max_val[ch];
                min_val[ch] = temp < min_val[ch] ? temp : min_val[ch];
            }
        }

        for (int i = 0; i < im.w; ++i)
        {
            for (int j = 0; j < im.h; ++j)
            {
                range = max_val[ch] - min_val[ch];

                set_pixel(im, i, j, ch, range != 0 ? (get_pixel(im, i, j, ch) - min_val[ch]) / range : 0);
            }
        }
    }
}

image *sobel_image(image im)
{
    image *mag_dir = calloc(2, sizeof(image));
    float x, y;
    mag_dir[0] = make_image(im.w, im.h, 1);
    mag_dir[1] = make_image(im.w, im.h, 1);

    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();

    image gx_out = convolve_image(im, gx_filter, 0);
    image gy_out = convolve_image(im, gy_filter, 0);

    for (int i = 0; i < im.w; ++i)
    {
        for (int j = 0; j < im.h; ++j)
        {

            x = get_pixel(gx_out, i, j, 0);
            y = get_pixel(gy_out, i, j, 0);
            set_pixel(mag_dir[0], i, j, 0, sqrt(x * x + y * y));
            set_pixel(mag_dir[1], i, j, 0, atan2(y, x));
        }
    }

    return mag_dir;
}

image colorize_sobel(image im)
{
    image *mag_and_direct = sobel_image(im);
    image colorize_sobel_ = make_image(im.w, im.h, im.c);
    feature_normalize(mag_and_direct[0]);
    feature_normalize(mag_and_direct[1]);
    float S, V, H;
    for (int w = 0; w < im.w; w++)
    {
        for (int h = 0; h < im.h; h++)
        {
            S = get_pixel(mag_and_direct[0], w, h, 0);
            V = get_pixel(mag_and_direct[0], w, h, 0);
            H = get_pixel(mag_and_direct[1], w, h, 0);
            set_pixel(colorize_sobel_, w, h, 0, H);
            set_pixel(colorize_sobel_, w, h, 1, S);
            set_pixel(colorize_sobel_, w, h, 2, V);
        }
    }
    hsv_to_rgb(colorize_sobel_);
    return colorize_sobel_;
}
