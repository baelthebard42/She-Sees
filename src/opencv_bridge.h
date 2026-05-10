#ifndef OPENCV_BRIDGE_H
#define OPENCV_BRIDGE_H

#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

void* open_video_stream(int cam_id);
image get_image_from_stream(void* cap);
int show_image(image im, const char *name, int ms);
void close_video_stream(void* cap);

#ifdef __cplusplus
}
#endif

#endif