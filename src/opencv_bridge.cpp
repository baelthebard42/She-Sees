#include <opencv2/opencv.hpp>
#include <cstring>
#include "image.h"

using namespace cv;

// -----------------------------
// Convert cv::Mat -> image
// Assumes float image in RGB format
// -----------------------------
static image mat_to_image(Mat& frame)
{
    Mat rgb;

    if (frame.channels() == 3)
        cvtColor(frame, rgb, COLOR_BGR2RGB);
    else
        cvtColor(frame, rgb, COLOR_GRAY2RGB);

    image im = make_image(rgb.cols, rgb.rows, 3);

    for (int y = 0; y < rgb.rows; y++)
    {
        for (int x = 0; x < rgb.cols; x++)
        {
            Vec3b pix = rgb.at<Vec3b>(y, x);

            set_pixel(im, x, y, 0, pix[0] / 255.0f);
            set_pixel(im, x, y, 1, pix[1] / 255.0f);
            set_pixel(im, x, y, 2, pix[2] / 255.0f);
        }
    }

    return im;
}

extern "C"
{

// -----------------------------
// Open camera
// -----------------------------
void* open_video_stream(int cam_id)
{
    VideoCapture* cap = new VideoCapture(cam_id);

    if (!cap->isOpened())
    {
        delete cap;
        return nullptr;
    }

    return (void*)cap;
}



// -----------------------------
// Capture frame
// -----------------------------
image get_image_from_stream(void* cap_ptr)
{
    VideoCapture* cap = (VideoCapture*)cap_ptr;

    Mat frame;
    (*cap) >> frame;

    if (frame.empty())
    {
        image empty = {0, 0, 0, nullptr};
        return empty;
    }

    return mat_to_image(frame);
}

// -----------------------------
// Display image
// -----------------------------
int show_image(image im, const char* name, int ms)
{
    if (!im.data) return -1;

    Mat display(im.h, im.w, CV_8UC3);

    for (int y = 0; y < im.h; y++)
    {
        for (int x = 0; x < im.w; x++)
        {
            Vec3b& pix = display.at<Vec3b>(y, x);

            pix[0] = (unsigned char)(get_pixel(im, x, y, 2) * 255);
            pix[1] = (unsigned char)(get_pixel(im, x, y, 1) * 255);
            pix[2] = (unsigned char)(get_pixel(im, x, y, 0) * 255);
        }
    }

    imshow(name, display);

    return waitKey(ms);
}

// -----------------------------
// Cleanup camera
// -----------------------------
void close_video_stream(void* cap_ptr)
{
    VideoCapture* cap = (VideoCapture*)cap_ptr;

    if (cap)
    {
        cap->release();
        delete cap;
    }
}

}