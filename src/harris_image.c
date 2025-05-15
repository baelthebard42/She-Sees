#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include <time.h>

#define INVALID_CORNER -999999

// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        free(d[i].data);
    }
    free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i)
{
    int w = 5;
    descriptor d;
    d.p.x = i % im.w;
    d.p.y = i / im.w;
    d.data = calloc(w * w * im.c, sizeof(float));
    d.n = w * w * im.c;
    int c, dx, dy;
    int count = 0;

    for (c = 0; c < im.c; ++c)
    {
        float cval = im.data[c * im.w * im.h + i];
        for (dx = -w / 2; dx < (w + 1) / 2; ++dx)
        {
            for (dy = -w / 2; dy < (w + 1) / 2; ++dy)
            {
                float val = get_pixel(im, i % im.w + dx, i / im.w + dy, c);
                d.data[count++] = cval - val;
            }
        }
    }
    return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p)
{
    int x = p.x;
    int y = p.y;
    int i;
    for (i = -9; i < 10; ++i)
    {
        set_pixel(im, x + i, y, 0, 1);
        set_pixel(im, x, y + i, 0, 1);
        set_pixel(im, x + i, y, 1, 0);
        set_pixel(im, x, y + i, 1, 0);
        set_pixel(im, x + i, y, 2, 1);
        set_pixel(im, x, y + i, 2, 1);
    }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n)
{
    int i;
    for (i = 0; i < n; ++i)
    {
        mark_spot(im, d[i].p);
    }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// row: defines whether we want row (1 x N) or column (N x 1)
// returns: single row or column image of the filter.
image make_1d_gaussian(float sigma, int row)
{

    assert(row == 0 || row == 1);
    int filter_size = 2 * (int)ceil(3 * sigma) + 1;

    image gauss_1d;

    if (row)
    {
        gauss_1d = make_image(filter_size, 1, 1);
    }
    else
    {
        gauss_1d = make_image(1, filter_size, 1);
    }

    for (int i = 0; i < filter_size; ++i)
    {
        int x = i - filter_size / 2; // relative weight from center

        if (row)
        {
            set_pixel(gauss_1d, i, 0, 0, gaussian(x, 0, sigma));
        }
        else
        {
            set_pixel(gauss_1d, 0, i, 0, gaussian(0, x, sigma));
        }
    }
    l1_normalize(gauss_1d);
    return gauss_1d;
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma, int use_1d_gauss)
{

    assert(use_1d_gauss == 1 || use_1d_gauss == 0);
    if (!use_1d_gauss)
    {
        image g = make_gaussian_filter(sigma);
        image s = convolve_image(im, g, 1);
        free_image(g);
        return s;
    }
    else // this makes filtering faster xD
    {
        image g_row = make_1d_gaussian(sigma, 1);
        image g_col = make_1d_gaussian(sigma, 0);

        image interm = convolve_image(im, g_row, 1);
        image final = convolve_image(interm, g_col, 1);

        free_image(g_row);
        free_image(g_col);
        free_image(interm);
        return final;
    }
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma)
{

    assert(im.c == 3 || im.c == 1);
    image S = make_image(im.w, im.h, 3);
    image sobel_x_filter = make_gx_filter();
    image sobel_y_filter = make_gy_filter();

    image intensity_x = rgb_to_grayscale(convolve_image(im, sobel_x_filter, 1)); // taking to grayscale since we need single channel to calculate required intensity values
    image intensity_y = rgb_to_grayscale(convolve_image(im, sobel_y_filter, 1)); // same

    float temp;

    for (int i = 0; i < S.w; ++i)
    {
        for (int j = 0; j < S.h; ++j)
        {
            for (int ch = 0; ch < S.c; ++ch)
            {

                if (ch == 0)
                {
                    temp = get_pixel(intensity_x, i, j, 0) * get_pixel(intensity_x, i, j, 0);
                }

                else if (ch == 1)
                {
                    temp = get_pixel(intensity_y, i, j, 0) * get_pixel(intensity_y, i, j, 0);
                }

                else
                {
                    temp = get_pixel(intensity_x, i, j, 0) * get_pixel(intensity_y, i, j, 0);
                }
                set_pixel(S, i, j, ch, temp);
            }
        }
    }

    S = smooth_image(S, sigma, 1); // the summation from the equation

    return S;
}

// Estimate the cornerness of each pixel given a structure matrix S (based on eigen values. here we basically approximate them using equation det(S) - alpha * trace(S)^2, alpha = .06.).
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S)
{
    float ALPHA = 0.06; // i wont make this tunable. go cry.
    image R = make_image(S.w, S.h, 1);

    for (int i = 0; i < S.w; ++i)
    {
        for (int j = 0; j < S.h; ++j)
        {

            float Ix_sq = get_pixel(S, i, j, 0);
            float Iy_sq = get_pixel(S, i, j, 1);
            float IxIy = get_pixel(S, i, j, 2);
            float det = Ix_sq * Iy_sq - IxIy * IxIy;
            float trace = Ix_sq + Iy_sq;
            float result = det - ALPHA * trace * trace;

            set_pixel(R, i, j, 0, result);
        }
    }
    return R;
}

int checkNeighbourPixels(image im, int i, int j, int w)
{

    float center = get_pixel(im, i, j, 0);

    for (int k = i - w; k <= i + w; ++k)
    {
        for (int l = j - w; l <= j + w; ++l)
        {

            if (k < 0 || k >= im.w || l < 0 || l >= im.h)
            {
                continue;
            }

            if (k == i && l == j)
            {
                continue;
            }

            if (get_pixel(im, k, l, 0) > center)
            {
                return INVALID_CORNER;
            }
        }
    }
    return 1;
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w)
{
    image r = copy_image(im);

    for (int i = 0; i < im.w; ++i)
    {
        for (int j = 0; j < im.h; ++j)
        {

            int response = checkNeighbourPixels(im, i, j, w);

            if (response != 1)
            {
                set_pixel(r, i, j, 0, response);
            }
        }
    }
    return r;
}

int countResponses(image rNMS)
{
    int num_corners = 0;

    for (int i = 0; i < rNMS.w; ++i)
    {
        for (int j = 0; j < rNMS.h; ++j)
        {
            if (get_pixel(rNMS, i, j, 0) != INVALID_CORNER)
                ++num_corners;
        }
    }

    return num_corners;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n)
{

    image S = structure_matrix(im, sigma); // done.

    image R = cornerness_response(S); // done. actually here we are supposed to calculate eigen values but we have settled with their approximations instead

    float max_cornerness = -INFINITY;
    for (int i = 0; i < R.w * R.h; ++i)
    {
        if (R.data[i] != INVALID_CORNER && R.data[i] > max_cornerness)
        {
            max_cornerness = R.data[i];
        }
    }
    printf("Max cornerness before thresholding: %f\n", max_cornerness);
    thresh = 0.01 * max_cornerness;

    for (int i = 0; i < R.w; ++i)
    {
        for (int j = 0; j < R.h; ++j)
        {
            float val = get_pixel(R, i, j, 0);
            if (val < thresh)
            {
                set_pixel(R, i, j, 0, INVALID_CORNER);
            }
        }
    }
    save_image(R, "cornerness_raw_after_low_marked");

    image Rnms = nms_image(R, nms); // done
    save_image(Rnms, "cornerness_nms");

    int count = countResponses(Rnms);

    *n = count;
    descriptor *d = calloc(count, sizeof(descriptor));

    int idx_desc = 0;
    for (int i = 0; i < Rnms.w; ++i)
    {
        for (int j = 0; j < Rnms.h; ++j)
        {
            if (get_pixel(Rnms, i, j, 0) != INVALID_CORNER)
            {
                d[idx_desc++] = describe_index(im, j * Rnms.w + i);
            }
        }
    }

    free_image(S);
    free_image(R);
    free_image(Rnms);
    return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms)
{
    int n = 0;
    descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
    mark_corners(im, d, n);
}
