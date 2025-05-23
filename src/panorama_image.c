#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"

// Comparator for matches
// const void *a, *b: pointers to the matches to compare.
// returns: result of comparison, 0 if same, 1 if a > b, -1 if a < b.
int match_compare(const void *a, const void *b)
{
    match *ra = (match *)a;
    match *rb = (match *)b;
    if (ra->distance < rb->distance)
        return -1;
    else if (ra->distance > rb->distance)
        return 1;
    else
        return 0;
}

// Place two images side by side on canvas, for drawing matching pixels.
// image a, b: images to place.
// returns: image with both a and b side-by-side.
image both_images(image a, image b)
{
    image both = make_image(a.w + b.w, a.h > b.h ? a.h : b.h, a.c > b.c ? a.c : b.c);
    int i, j, k;
    for (k = 0; k < a.c; ++k)
    {
        for (j = 0; j < a.h; ++j)
        {
            for (i = 0; i < a.w; ++i)
            {
                set_pixel(both, i, j, k, get_pixel(a, i, j, k));
            }
        }
    }
    for (k = 0; k < b.c; ++k)
    {
        for (j = 0; j < b.h; ++j)
        {
            for (i = 0; i < b.w; ++i)
            {
                set_pixel(both, i + a.w, j, k, get_pixel(b, i, j, k));
            }
        }
    }
    return both;
}

// Draws lines between matching pixels in two images.
// image a, b: two images that have matches.
// match *matches: array of matches between a and b.
// int n: number of matches.
// int inliers: number of inliers at beginning of matches, drawn in green.
// returns: image with matches drawn between a and b on same canvas.
image draw_matches(image a, image b, match *matches, int n, int inliers)
{
    image both = both_images(a, b);
    int i, j;
    for (i = 0; i < n; ++i)
    {
        int bx = matches[i].p.x;
        int ex = matches[i].q.x;
        int by = matches[i].p.y;
        int ey = matches[i].q.y;
        for (j = bx; j < ex + a.w; ++j)
        {
            int r = (float)(j - bx) / (ex + a.w - bx) * (ey - by) + by;
            set_pixel(both, j, r, 0, i < inliers ? 0 : 1);
            set_pixel(both, j, r, 1, i < inliers ? 1 : 0);
            set_pixel(both, j, r, 2, 0);
        }
    }
    return both;
}

// Draw the matches with inliers in green between two images.
// image a, b: two images to match.
// matches *
image draw_inliers(image a, image b, matrix H, match *m, int n, float thresh)
{
    int inliers = model_inliers(H, m, n, thresh);
    image lines = draw_matches(a, b, m, n, inliers);
    return lines;
}

// Find corners, match them, and draw them between two images.
// image a, b: images to match.
// float sigma: gaussian for harris corner detector. Typical: 2
// float thresh: threshold for corner/no corner. Typical: 1-5
// int nms: window to perform nms on. Typical: 3
image find_and_draw_matches(image a, image b, float sigma, float thresh, int nms)
{
    int an = 0;
    int bn = 0;
    int mn = 0;
    descriptor *ad = harris_corner_detector(a, sigma, thresh, nms, &an);
    descriptor *bd = harris_corner_detector(b, sigma, thresh, nms, &bn);
    match *m = match_descriptors(ad, an, bd, bn, &mn);

    mark_corners(a, ad, an);
    mark_corners(b, bd, bn);
    image lines = draw_matches(a, b, m, mn, 0);

    free_descriptors(ad, an);
    free_descriptors(bd, bn);
    free(m);
    return lines;
}

// Calculates L1 distance between to floating point arrays.
// float *a, *b: arrays to compare.
// int n: number of values in each array.
// returns: l1 distance between arrays (sum of absolute differences).
float l1_distance(float *a, float *b, int n)
{
    float l1_summed = 0;
    float l1_dist;

    for (int i = 0; i < n; ++i)
    {

        l1_dist = a[i] - b[i];

        if (l1_dist < 0)
            l1_dist = (-1) * l1_dist; // by definition of absolute values

        l1_summed = l1_summed + l1_dist;
    }
    return l1_summed;
}

/*void shift_to_left(match *m, int n,  int start_index){

    for (int i = start_index; i<n; ++i){
        m[i-1]=m[i];
    }

} chaiyo vane chalaula*/

// Finds best matches between descriptors of two images.
// descriptor *a, *b: array of descriptors for pixels in two images.
// int an, bn: number of descriptors in arrays a and b.
// int *mn: pointer to number of matches found, to be filled in by function.
// returns: best matches found. each descriptor in a should match with at most
//          one other descriptor in b.
match *match_descriptors(descriptor *a, int an, descriptor *b, int bn, int *mn)
{
    int i, j;

    // We will have at most an matches.
    *mn = an;
    match *m = calloc(an, sizeof(match));
    for (j = 0; j < an; ++j)
    {
        // TODO: for every descriptor in a, find best match in b.
        // record ai as the index in *a and bi as the index in *b.
        int bind = 0; // <- find the best match
        m[j].ai = j;
        m[j].bi = bind; // <- should be index in b.
        m[j].p = a[j].p;
        m[j].q = b[bind].p;
        m[j].distance = 99999999; // <- should be the smallest L1 distance!

        for (i = 0; i < bn; ++i)
        {

            float l1_dist = l1_distance(a[j].data, b[i].data, a[j].n);

            if (l1_dist < m[j].distance)
            {
                m[j].bi = i;
                m[j].q = b[i].p;
                m[j].distance = l1_dist;
            }
        }
    }

    int count = 0;
    int *seen = calloc(bn, sizeof(int));

    qsort(m, an, sizeof(match), match_compare);

    for (int i = 0; i < an; ++i)
    {

        if (!seen[m[i].bi]) // 4000 iq move
        {
            seen[m[i].bi] = 1;
            m[count++] = m[i];
        }
    }

    *mn = count;
    m = realloc(m, count * sizeof(match)); // this effectively removes the elements that arent needed by reallocating less memory
    free(seen);
    return m;
}

// Apply a projective transformation to a point.
// matrix H: homography to project point.
// point p: point to project.
// returns: point projected using the homography.
point project_point(matrix H, point p)
{
    matrix c = make_matrix(3, 1);

    c.data[0][0] = p.x; // representing the point in matirx form
    c.data[1][0] = p.y;
    c.data[2][0] = 1;
    matrix result = matrix_mult_matrix(H, c);

    float factor = result.data[2][0]; // this represent the w values

    if (factor == 0)
    {
        return make_point(0, 0);
    }

    point q = make_point(result.data[0][0] / factor, result.data[1][0] / factor);
    return q;
}

// Calculate L2 distance between two points.
// point p, q: points.
// returns: L2 distance between them.
float point_distance(point p, point q)
{
    return sqrt(powf(p.x - q.x, 2) + powf(p.y - q.y, 2));
}

void swap(void *a, void *b, size_t size)
{
    unsigned char temp[size];
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

// Count number of inliers in a set of matches. Should also bring inliers
// to the front of the array.
// matrix H: homography between coordinate systems.
// match *m: matches to compute inlier/outlier.
// int n: number of matches in m.
// float thresh: threshold to be an inlier.
// returns: number of inliers whose projected point falls within thresh of
//          their match in the other image. Should also rearrange matches
//          so that the inliers are first in the array. For drawing.
int model_inliers(matrix H, match *m, int n, float thresh)
{
    int count = 0;

    for (int i = 0; i < n; ++i)
    {
        if (point_distance(project_point(H, m[i].p), m[i].q) < thresh)
            swap(&m[i], &m[count++], sizeof(match)); // we also need to sort. this effectively sorts the list such that inliers will be ahead of outliers
    }
    return count;
}

int get_random_number(int min, int max)
{

    return rand() % (max - min + 1) + min;
}

// Randomly shuffle matches for RANSAC.
// match *m: matches to shuffle in place.
// int n: number of elements in matches.
void randomize_matches(match *m, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        swap(&m[i], &m[get_random_number(0, i)], sizeof(match));
    }
}

// Computes homography between two images given matching pixels.
// match *matches: matching points between images.
// int n: number of matches to use in calculating homography.
// returns: matrix representing homography H that maps image a to image b.
matrix compute_homography(match *matches, int n)
{
    matrix M = make_matrix(n * 2, 8);
    matrix b = make_matrix(n * 2, 1);

    int i;
    for (i = 0; i < n; ++i)
    {
        double x = matches[i].p.x;
        double xp = matches[i].q.x;
        double y = matches[i].p.y;
        double yp = matches[i].q.y;

        // this is based on equations of modeling the projection in form of M*a=b
        M.data[2 * i][0] = x;
        M.data[2 * i][1] = y;
        M.data[2 * i][2] = 1;
        M.data[2 * i][3] = 0;
        M.data[2 * i][4] = 0;
        M.data[2 * i][5] = 0;
        M.data[2 * i][6] = -x * xp;
        M.data[2 * i][7] = -y * xp;
        b.data[2 * i][0] = xp;

        M.data[2 * i + 1][0] = 0;
        M.data[2 * i + 1][1] = 0;
        M.data[2 * i + 1][2] = 0;
        M.data[2 * i + 1][3] = x;
        M.data[2 * i + 1][4] = y;
        M.data[2 * i + 1][5] = 1;
        M.data[2 * i + 1][6] = -x * yp;
        M.data[2 * i + 1][7] = -y * yp;
        b.data[2 * i + 1][0] = yp;
    }
    matrix a = solve_system(M, b);
    free_matrix(M);
    free_matrix(b);

    matrix none = {0}; // if a solution can't be found, return empty matrix;
    if (!a.data)
        return none;

    matrix H = make_matrix(3, 3);
    int counter = 0;

    for (int i = 0; i < H.rows; ++i)
    {
        for (int j = 0; j < H.cols; ++j)
        {
            H.data[i][j] = a.data[counter++][0];
        }
    }
    free_matrix(a);
    return H;
}

// Perform RANdom SAmple Consensus to calculate homography for noisy matches.
// match *m: set of matches.
// int n: number of matches.
// float thresh: inlier/outlier distance threshold.
// int k: number of iterations to run.
// int cutoff: inlier cutoff to exit early.
// returns: matrix representing most common homography between matches.
matrix RANSAC(match *m, int n, float thresh, int k, int cutoff)
{
    int e;
    int best = 0;
    matrix Hb = make_translation_homography(256, 0);

    int num_points_to_fit = 4; // rule of thumb for fitting homography matrices

    for (int i = 0; i < k; ++i)
    {
        randomize_matches(m, n);

        match *m_sliced = calloc(num_points_to_fit, sizeof(match));

        for (int j = 0; j < num_points_to_fit; ++j)
        {
            m_sliced[j] = m[j];
        }

        matrix predicted_homography = compute_homography(m_sliced, num_points_to_fit);
        int num_inliers = model_inliers(predicted_homography, m, n, thresh);

        if (num_inliers > best)
        {

            if (num_inliers > cutoff)
            {
                free(m_sliced);
                return predicted_homography;
            }

            best = num_inliers;
            free_matrix(Hb);
            Hb = copy_matrix(predicted_homography);
        }
    }

    return Hb;
}

// Stitches two images together using a projective transformation.
// image a, b: images to stitch.
// matrix H: homography from image a coordinates to image b coordinates.
// returns: combined image stitched together.
image combine_images(image a, image b, matrix H)
{
    matrix Hinv = matrix_invert(H);

    // Project the corners of image b into image a coordinates.
    point c1 = project_point(Hinv, make_point(0, 0));
    point c2 = project_point(Hinv, make_point(b.w - 1, 0));
    point c3 = project_point(Hinv, make_point(0, b.h - 1));
    point c4 = project_point(Hinv, make_point(b.w - 1, b.h - 1));

    // Find top left and bottom right corners of image b warped into image a.
    point topleft, botright;
    botright.x = MAX(c1.x, MAX(c2.x, MAX(c3.x, c4.x)));
    botright.y = MAX(c1.y, MAX(c2.y, MAX(c3.y, c4.y)));
    topleft.x = MIN(c1.x, MIN(c2.x, MIN(c3.x, c4.x)));
    topleft.y = MIN(c1.y, MIN(c2.y, MIN(c3.y, c4.y)));

    int dx = MIN(0, topleft.x);
    int dy = MIN(0, topleft.y);
    int w = MAX(a.w, botright.x) - dx;
    int h = MAX(a.h, botright.y) - dy;

    if (w > 7000 || h > 7000) // can disable this if making very big panoramas
    {
        fprintf(stderr, "output too big, stopping\n");
        return copy_image(a);
    }

    int i, j, k;
    image c = make_image(w, h, a.c);

    // Paste image a into the new image offset by dx and dy.
    for (k = 0; k < a.c; ++k)
    {
        for (j = 0; j < a.h; ++j)
        {
            for (i = 0; i < a.w; ++i)
            {
                set_pixel(c, i - dx, j - dy, k, get_pixel(a, i, j, k));
            }
        }
    }

    for (j = 0; j < c.h; ++j)
    {
        for (i = 0; i < c.w; ++i)
        {

            point p = make_point(i + dx, j + dy);

            point pb = project_point(H, p);

            if (pb.x >= 0 && pb.x < b.w - 1 && pb.y >= 0 && pb.y < b.h - 1)
            {
                for (k = 0; k < b.c; ++k)
                {
                    float val = bilinear_interpolate(b, pb.x, pb.y, k);
                    set_pixel(c, i, j, k, val);
                }
            }
        }
    }
    return c;
}

// Create a panoramam between two images.
// image a, b: images to stitch together.
// float sigma: gaussian for harris corner detector. Typical: 2
// float thresh: threshold for corner/no corner. Typical: 1-5
// int nms: window to perform nms on. Typical: 3
// float inlier_thresh: threshold for RANSAC inliers. Typical: 2-5
// int iters: number of RANSAC iterations. Typical: 1,000-50,000
// int cutoff: RANSAC inlier cutoff. Typical: 10-100
image panorama_image(image a, image b, float sigma, float thresh, int nms, float inlier_thresh, int iters, int cutoff)
{

    printf("i have begun the panoroma");
    assert(a.data != NULL);
    assert(b.data != NULL);

    srand(10);
    int an = 0;
    int bn = 0;
    int mn = 0;

    // Calculate corners and descriptors
    descriptor *ad = harris_corner_detector(a, sigma, thresh, nms, &an);
    descriptor *bd = harris_corner_detector(b, sigma, thresh, nms, &bn);

    if (!ad || !bd)
    {
        fprintf(stderr, "Descriptor generation failed.\n");
        return make_image(1, 1, 1); // or appropriate error handling
    }

    // Find matches
    match *m = match_descriptors(ad, an, bd, bn, &mn);

    if (!m || mn == 0)
    {
        fprintf(stderr, "Descriptor matching failed.\n");
        return make_image(1, 1, 1); // or appropriate error handling
    }

    // Run RANSAC to find the homography
    matrix H = RANSAC(m, mn, inlier_thresh, iters, cutoff);

    if (1)
    {
        // Mark corners and matches between images
        mark_corners(a, ad, an);
        mark_corners(b, bd, bn);
        image inlier_matches = draw_inliers(a, b, H, m, mn, inlier_thresh);
        save_image(inlier_matches, "inliers");
    }

    free_descriptors(ad, an);
    free_descriptors(bd, bn);
    free(m);

    // Stitch the images together with the homography
    image comb = combine_images(a, b, H);
    return comb;
}
