#ifndef IMAGE_H
#define IMAGE_H
#include "matrix.h"
#define TWOPI 6.2831853
#include <math.h>

// you dont want to edit anything in this file

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifdef __cplusplus
extern "C"
{
#endif

    static float gaussian(float x, float y, float sigma)
    {
        return (1 / (TWOPI * sigma * sigma)) * exp(-((x * x + y * y) / (2 * (sigma * sigma))));
    }

    typedef struct
    {
        int w, h, c;
        float *data;
    } image;

    typedef struct
    {
        float x, y;
    } point;

    typedef struct
    {
        point p;
        int n;
        float *data;
    } descriptor;

    typedef struct
    {
        point p, q;
        int ai, bi;
        float distance;
    } match;

    static point make_point(float x, float y)
    {
        point p;
        p.x = x;
        p.y = y;
        return p;
    }

    float get_pixel(image im, int x, int y, int c);
    void set_pixel(image im, int x, int y, int c, float v);
    image copy_image(image im);
    image rgb_to_grayscale(image im);
    image grayscale_to_rgb(image im, float r, float g, float b);
    void rgb_to_hsv(image im);
    void hsv_to_rgb(image im);
    void shift_image(image im, int c, float v);
    void scale_image(image im, int c, float v);
    void clamp_image(image im);
    image get_channel(image im, int c);
    int same_image(image a, image b);
    image sub_image(image a, image b);
    image add_image(image a, image b);

    // loading and saving
    image make_image(int w, int h, int c);
    image load_image(char *filename);
    void save_image(image im, const char *name);
    void save_png(image im, const char *name);
    void free_image(image im);

    // resizing
    float nn_interpolate(image im, float x, float y, int c);
    image nn_resize(image im, int w, int h);
    float bilinear_interpolate(image im, float x, float y, int c);
    image bilinear_resize(image im, int w, int h);

    // filtering
    image convolve_image(image im, image filter, int preserve);
    image make_box_filter(int w);
    image make_highpass_filter();
    image make_sharpen_filter();
    image make_emboss_filter();
    image make_gaussian_filter(float sigma);
    image make_gx_filter();
    image make_gy_filter();
    void feature_normalize(image im);
    void l1_normalize(image im);
    void threshold_image(image im, float thresh);
    image *sobel_image(image im);
    image colorize_sobel(image im);
    image smooth_image(image im, float sigma, int use_1d_gauss);

    // panoroma, corner detection
    image structure_matrix(image im, float sigma);
    image cornerness_response(image S);
    void free_descriptors(descriptor *d, int n);
    void mark_corners(image im, descriptor *d, int n);
    image find_and_draw_matches(image a, image b, float sigma, float thresh, int nms);
    void detect_and_draw_corners(image im, float sigma, float thresh, int nms);
    int model_inliers(matrix H, match *m, int n, float thresh);
    image combine_images(image a, image b, matrix H);
    match *match_descriptors(descriptor *a, int an, descriptor *b, int bn, int *mn);
    descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n);
    image panorama_image(image a, image b, float sigma, float thresh, int nms, float inlier_thresh, int iters, int cutoff);

    // optical flow
    image make_integral_image(image im);
    image box_filter_image(image im, int s);
    image time_structure_matrix(image im, image prev, int s);
    image velocity_image(image S, int stride);
    image optical_flow_images(image im, image prev, int smooth, int stride);
    void optical_flow_webcam(int smooth, int stride, int div);
    void draw_flow(image im, image v, float scale);

#ifdef OPENCV
    void *open_video_stream(const char *f, int c, int h, int w, int fps);
    image get_image_from_stream(void *p);
    void make_window(char *name, int h, int w, int fullscreen);
    int show_image(image im, const char *name, int ms);
#endif

#ifdef __cplusplus
}
#endif

#endif
