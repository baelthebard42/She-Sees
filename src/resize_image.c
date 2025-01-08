#include <math.h>
#include "image.h"
#include "process_image.c"
#include <string.h>

float rectangle_area_from_diag(float x1, float y1, float x2, float y2)
{
    return abs((x2 - x1) * (y2 - y1));
}

float nn_interpolate(image im, float x, float y, int c)
{
    assert(x >= 0 && x < im.w && y >= 0 && y < im.h);
    return get_pixel(im, round(x), round(y), c);
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    assert(x >= 0 && x < im.w && y >= 0 && y < im.h);
    int close_neighbor_x[4], close_neighbor_y[4];

    close_neighbor_x[0] = (int)x, close_neighbor_y[0] = (int)y;
    close_neighbor_x[1] = (int)x + 1, close_neighbor_y[1] = (int)y;
    close_neighbor_x[2] = (int)x, close_neighbor_y[2] = (int)y + 1;
    close_neighbor_x[3] = (int)x + 1, close_neighbor_y[3] = (int)y + 1;

    float areas[4], total_area = rectangle_area_from_diag(close_neighbor_x[0], close_neighbor_y[0], close_neighbor_x[3], close_neighbor_y[3]);
    float result_pixel = 0;

    for (int i = 0; i < 4; ++i)
    {
        areas[3 - i] = rectangle_area_from_diag(x, y, close_neighbor_x[i], close_neighbor_y[i]) / total_area;
        result_pixel += areas[3 - i] * get_pixel(im, close_neighbor_x[3 - i], close_neighbor_y[3 - i], c);
    }

    return result_pixel;
}

image resize(image im, int w, int h, char interpolation)
{

    assert(w > 0 && h > 0 && (strcmp(interpolation, "b") == 0 || strcmp(interpolation, "n") == 0));

    image resized_image = make_image(w, h, im.c);

    float new_to_old_ratio_w = (float)resized_image.w / im.w;
    float new_to_old_ratio_h = (float)resized_image.h / im.h;
    float predicted_pixel;

    resized_image.data = (float *)(malloc(resized_image.c * resized_image.w * resized_image.h * sizeof(float)));

    for (int i = 0; i < resized_image.w; ++i)
    {
        for (int j = 0; j < resized_image.h; ++j)
        {

            float old_coord_x = i / new_to_old_ratio_w;
            float old_coord_y = j / new_to_old_ratio_h;

            for (int ch = 0; ch < resized_image.c; ++ch)
            {
                if (strcmp(interpolation, "b") == 0)
                {
                    predicted_pixel = bilinear_interpolate(im, old_coord_x, old_coord_y, ch);
                }

                else
                {
                    predicted_pixel = nn_interpolate(im, old_coord_x, old_coord_y, ch);
                }

                set_pixel(resized_image, i, j, ch, predicted_pixel);
            }
        }
    }

    return resized_image;
}
