#define OPENCV
#include "image.h"

int main()
{
    // You can tweak these values:
    int smooth = 2; // Gaussian blur strength (higher = more smoothing)
    int stride = 4; // Distance between flow vectors
    int div = 8;    // Subsampling for drawing flow (higher = fewer arrows)

    optical_flow_webcam(smooth, stride, div);
    return 0;
}
