#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include "opencv_bridge.h"


// Note: This implementation of Lucas-Kanade optical flow DOES NOT 
// iterate over all local neighborhood pixel as explained by the equations
// of optical flow constraints. It instead smooths the time structure matrix which
// ensures the neighboring information is encoded in it already.
// Then during the optical flow computation, the motion vector [u, v] is calculated only for
// a representative pixel of the neighborhood (see velocity_image method)

// Draws a line on an image with color corresponding to the direction of line
// image im: image to draw line on
// float x, y: starting point of line
// float dx, dy: vector corresponding to line angle and magnitude
void draw_line(image im, float y, float x, float dy, float dx)
{
    assert(im.c == 3);
    float angle = 6 * (atan2(dy, dx) / TWOPI + .5);
    int index = floor(angle);
    float f = angle - index;
    float r, g, b;
    if (index == 0)
    {
        r = 1;
        g = f;
        b = 0;
    }
    else if (index == 1)
    {
        r = 1 - f;
        g = 1;
        b = 0;
    }
    else if (index == 2)
    {
        r = 0;
        g = 1;
        b = f;
    }
    else if (index == 3)
    {
        r = 0;
        g = 1 - f;
        b = 1;
    }
    else if (index == 4)
    {
        r = f;
        g = 0;
        b = 1;
    }
    else
    {
        r = 1;
        g = 0;
        b = 1 - f;
    }
    float i;
    float d = sqrt(dx * dx + dy * dy);
    for (i = 0; i < d; i += 1)
    {
        int xi = x + dx * i / d;
        int yi = y + dy * i / d;
        set_pixel(im, xi, yi, 0, r);
        set_pixel(im, xi, yi, 1, g);
        set_pixel(im, xi, yi, 2, b);
    }
}

// Make an integral image or summed area table from an image
// image im: image to process
// returns: image I such that I[x,y] = sum{i<=x, j<=y}(im[i,j])
image make_integral_image(image im)
{
    image integ = make_image(im.w, im.h, im.c);

    for (int i = 0; i < integ.w; ++i)
    {
        for (int j = 0; j < integ.h; ++j)
        {
            for (int ch = 0; ch < integ.c; ++ch)
            {

                float current_pix = get_pixel(im, i, j, ch);
                float left_pix = (i > 0) ? get_pixel(integ, i - 1, j, ch) : 0;
                float above_pix = (j > 0) ? get_pixel(integ, i, j - 1, ch) : 0;
                float left_and_above_pix = (i > 0 && j > 0) ? get_pixel(integ, i - 1, j - 1, ch) : 0;

                float integral_sum = current_pix + left_pix + above_pix - left_and_above_pix; // subtracted to fix the overlapping area problem

                set_pixel(integ, i, j, ch, integral_sum);
            }
        }
    }

    return integ;
}

float get_sum_with_integral_image(image integ, int topLeft_x, int topLeft_y, int bottomRight_x, int bottomRight_y, int c)
{
    return get_pixel(integ, bottomRight_x, bottomRight_y, c) - get_pixel(integ, topLeft_x - 1, bottomRight_y, c) - get_pixel(integ, bottomRight_x, topLeft_y - 1, c) + get_pixel(integ, topLeft_x - 1, topLeft_y - 1, c);
}

// Apply a box filter to an image using an integral image for speed
// image im: image to smooth
// int s: window size for box filter
// returns: smoothed image
image box_filter_image(image im, int s)
{
    assert(s >= 0 && s <= im.w && s <= im.h);
    image integ = make_integral_image(im);
    image S = make_image(im.c, im.h, im.w);

    for (int i = 0; i < im.w; ++i)
    {
        for (int j = 0; j < im.h; ++j)
        {
            for (int ch = 0; ch < im.c; ++ch)
            {

                float topLeft_x = i - s >= 0 ? i - s : 0;
                float topLeft_y = j - s >= 0 ? j - s : 0;
                float bottomRight_x = i + s < im.w ? i + s : im.w - 1;
                float bottomRight_y = j + s < im.h ? j + s : im.h - 1;
                int area = (bottomRight_x - topLeft_x + 1) * (bottomRight_y - topLeft_y + 1);
                float sum = get_sum_with_integral_image(integ, topLeft_x, topLeft_y, bottomRight_x, bottomRight_y, ch);
                sum = sum / area;
                if (sum > 1)
                {
                    sum = 1;
                }
                set_pixel(S, i, j, ch, sum);
            }
        }
    }

    free_image(integ);
    return S;
}

// Calculate the time-structure matrix of an image pair.
// image im: the input image.
// image prev: the previous image in sequence.
// int s: window size for smoothing.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          3rd channel is IxIy, 4th channel is IxIt, 5th channel is IyIt. (I here represents image gradient not image intensity.)
image time_structure_matrix(image im, image prev, int s)
{
    int i;
    int converted = 0;
    if (im.c == 3)
    {
        converted = 1;
        im = rgb_to_grayscale(im);
        prev = rgb_to_grayscale(prev);
    }

    image time_gradients = sub_image(im, prev);

    float temp;
    image S = make_image(im.w, im.h, 5);
    image sobel_x_filter = make_gx_filter(); //essential in calculating the image gradients
    image sobel_y_filter = make_gy_filter();

    image intensity_x_grads = convolve_image(im, sobel_x_filter, 1);
    image intensity_y_grads = convolve_image(im, sobel_y_filter, 1);

    for (int i = 0; i < S.w; ++i)
    {
        for (int j = 0; j < S.h; ++j)
        {
            for (int ch = 0; ch < S.c; ++ch)
            {

                if (ch == 0)
                {
                    temp = get_pixel(intensity_x_grads, i, j, 0) * get_pixel(intensity_x_grads, i, j, 0);
                }
                else if (ch == 1)
                {
                    temp = get_pixel(intensity_y_grads, i, j, 0) * get_pixel(intensity_y_grads, i, j, 0);
                }

                else if (ch == 2)
                {
                    temp = get_pixel(intensity_x_grads, i, j, 0) * get_pixel(intensity_y_grads, i, j, 0);
                }

                else if (ch == 3)
                {
                    temp = get_pixel(intensity_x_grads, i, j, 0) * get_pixel(time_gradients, i, j, 0);
                }
                else
                {
                    temp = get_pixel(intensity_y_grads, i, j, 0) * get_pixel(time_gradients, i, j, 0);
                }

                set_pixel(S, i, j, ch, temp);
            }
        }
    }

    if (converted)
    {
        free_image(im);
        free_image(prev);
    }

    float sigma = s / 6; // rule of thumb

    S = smooth_image(S, sigma, 1); // each pixel gets info from neighboring pixels, useful for optical flow computation later
    return S;
}

// Calculate the velocity given a structure image
// image S: time-structure image (smoothed)
// int stride: motion vector or velocity is computed every stride pixels (not each pixel)
image velocity_image(image S, int stride)
{
    image v = make_image(S.w / stride, S.h / stride, 3);
    int i, j;
    matrix M = make_matrix(2, 2); // structure tensor or second moment matrix that encodes local grad geometry
    matrix p = make_matrix(2, 1); // time grad matrix for Ixt and Iyt

    // the intuition is to solve for vx, vy using information encoded in M and p
    // starting from (stride-1)/2 instead of 0 since it gives centre of the stride pixels. then each iteration gives centre of next block and so on
    // the computation occurs just for that representative pixel, not others
    for (j = (stride - 1) / 2; j < S.h; j += stride)
    {
        for (i = (stride - 1) / 2; i < S.w; i += stride)
        {
            float Ixx = S.data[i + S.w * j + 0 * S.w * S.h];
            float Iyy = S.data[i + S.w * j + 1 * S.w * S.h];
            float Ixy = S.data[i + S.w * j + 2 * S.w * S.h];
            float Ixt = S.data[i + S.w * j + 3 * S.w * S.h];
            float Iyt = S.data[i + S.w * j + 4 * S.w * S.h];

            float det = Ixx * Iyy - Ixy * Ixy; // invertibility check
            if (fabs(det) < 1e-6)
            {
                set_pixel(v, i / stride, j / stride, 0, 0);
                set_pixel(v, i / stride, j / stride, 1, 0);

                continue;
            }

            M.data[0][0] = Ixx;
            M.data[0][1] = Ixy;
            M.data[1][0] = Ixy;
            M.data[1][1] = Iyy;

            p.data[0][0] = -Ixt;
            p.data[1][0] = -Iyt;

            matrix Minv = matrix_invert(M);
            matrix solution = matrix_mult_matrix(Minv, p);

            float vx = solution.data[0][0];
            float vy = solution.data[1][0];

            free_matrix(Minv);
            free_matrix(solution);

            set_pixel(v, i / stride, j / stride, 0, vx);
            set_pixel(v, i / stride, j / stride, 1, vy);
        }
    }
    free_matrix(M);
    free_matrix(p);
    return v;
}

// Draw lines on an image given the velocity
// image im: image to draw on
// image v: velocity of each pixel
// float scale: scalar to multiply velocity by for drawing
void draw_flow(image im, image v, float scale)
{
    int stride = im.w / v.w;
    int i, j;
    for (j = (stride - 1) / 2; j < im.h; j += stride)
    {
        for (i = (stride - 1) / 2; i < im.w; i += stride)
        {
            float dx = scale * get_pixel(v, i/stride, j/stride, 0);
            float dy = scale * get_pixel(v, i/stride, j/stride, 1);
            if (fabs(dx) > im.w)
                dx = 0;
            if (fabs(dy) > im.h)
                dy = 0;
            draw_line(im, j, i, dy, dx);
        }
    }
}

// Constrain the absolute value of each image pixel
// image im: image to constrain
// float v: each pixel will be in range [-v, v]
void constrain_image(image im, float v)
{
    int i;
    for (i = 0; i < im.w * im.h * im.c; ++i)
    {
        if (im.data[i] < -v)
            im.data[i] = -v;
        if (im.data[i] > v)
            im.data[i] = v;
    }
}

// Calculate the optical flow between two images
// image im: current image
// image prev: previous image
// int smooth: amount to smooth structure matrix by
// int stride: downsampling for velocity matrix
// returns: velocity matrix
image optical_flow_images(image im, image prev, int smooth, int stride)
{
    image S = time_structure_matrix(im, prev, smooth);
    image v = velocity_image(S, stride);
    constrain_image(v, 6);
    image vs = smooth_image(v, 2, 0);
    free_image(v);
    free_image(S);
    return vs;
}


// Run optical flow demo on webcam
// smooth: structure matrix smoothing
// stride: velocity sampling stride
// div: downsampling factor
void optical_flow_webcam(int smooth, int stride, int div)
{
#ifdef OPENCV

    void *cap = open_video_stream(0);

    if (!cap)
    {
        fprintf(stderr, "Failed to open camera\n");
        return;
    }

    image prev = {0};
    image im = {0};

    // Wait for first valid frame
    while (!prev.data)
    {
        prev = get_image_from_stream(cap);

        if (!prev.data)
        {
            printf("Waiting for camera frame...\n");
        }
    }

    int rw = prev.w / div;
    int rh = prev.h / div;

    if (rw < 1) rw = 1;
    if (rh < 1) rh = 1;

    image prev_c = nn_resize(prev, rw, rh);

    // Wait for second valid frame
    while (!im.data)
    {
        im = get_image_from_stream(cap);

        if (!im.data)
        {
            printf("Waiting for second frame...\n");
        }
    }

     rw = im.w / div;
     rh = im.h / div;

    if (rw < 1) rw = 1;
    if (rh < 1) rh = 1;

  

    image im_c = nn_resize(im, rw, rh);

    while (1)
    {
        // Skip invalid frames instead of crashing
        if (!im.data)
        {
            im = get_image_from_stream(cap);
            continue;
        }

        image copy = copy_image(im);

        image v = optical_flow_images(im_c, prev_c, smooth, stride);

        draw_flow(copy, v, smooth * div);

        int key = show_image(copy, "flow", 5);

        free_image(v);
        free_image(copy);

        free_image(prev);
        free_image(prev_c);

        prev = im;
        prev_c = im_c;

        if (key == 27)
        {
            break;
        }

        // Keep trying until a valid frame appears
        do
        {
            im = get_image_from_stream(cap);

            if (!im.data)
            {
                printf("Dropped frame... retrying\n");
            }

        } while (!im.data);

        im_c = nn_resize(im, im.w/div, im.h/div);
    }

    close_video_stream(cap);

#else
    fprintf(stderr, "Must compile with OpenCV\n");
#endif
}