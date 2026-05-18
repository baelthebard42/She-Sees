# She-sees

She-Sees is a computer vision library written in C for both C and C++. The library is based entirely on classical computer vision techniques rather than modern machine learning based approaches.

The project was created primarily for learning purposes, but it can also be used for research and other computer vision activities in C.

It can also be used with Python, see `uwimg.py` for the Python bindings.

---

## Table of Contents

- [Installation](#installation)
- [Loading and Saving Images](#loading-and-saving-images)
- [Pixel-Level Access and Utility Functions](#pixel-level-access-and-utility-functions)
- [Color Manipulation](#color-manipulation)
- [Corner Detection and Panorama](#corner-detection-and-panorama)
  - [Harris Corner Detector](#harris-corner-detector)
  - [Panorama Stitching](#panorama-stitching)
- [Optical Flow](#optical-flow)
  - [optical_flow_images](#optical_flow_images)
  - [optical_flow_webcam](#optical_flow_webcam)
  - [Low-Level Operations](#low-level-operations)
- [Resizing](#resizing)
  - [Nearest Neighbor Resize](#nearest-neighbor-resize)
  - [Bilinear Resize](#bilinear-resize)
- [Filtering](#filtering)
  - [Convolution](#convolution)
  - [Available Filters](#available-filters)
  - [Normalization](#normalization)
  - [Sobel Edge Detection](#sobel-edge-detection)
  - [Image Smoothing](#image-smoothing)
- [Credits and Contributing](#credits-and-contributing)



## Installation

### Prerequisites

Make sure the following are installed on your system:

- `gcc` and `g++` compilers
- OpenCV 4.0 or newer

### Clone the Repository

```bash
git clone https://github.com/baelthebard42/She-Sees
cd She-Sees
```

### Build

```bash
make OPENCV=1
```

### Usage

Include `image.h` in your source file:

```c
#include "image.h"
```

When compiling your project, link against the necessary library files produced by the build step (check the obj directory in main directory of the library).

---

## Loading and Saving Images

### The `image` struct


All image data in She-sees is represented using the following struct:

```c
typedef struct {
    int w, h, c;
    float *data;
} image;
```

- `w` — width in pixels
- `h` — height in pixels
- `c` — number of channels (e.g. 3 for RGB)
- `data` — pixel data stored as floats in **row-major order**


---

### `make_image`

```c
image make_image(int w, int h, int c);
```

Allocates and returns a new image of the given dimensions. Pixel values are uninitialized.

---

### `load_image`

```c
image load_image(char *filename);
```

Loads an image from disk. Returns an `image` struct populated with the pixel data from the file. Supports common formats such as PNG and JPEG.

---

### `save_image`

```c
void save_image(image im, const char *name);
```

Saves the image `im` to disk. The output format is determined by the extension in `name`.

---

### `save_png`

```c
void save_png(image im, const char *name);
```

Saves the image `im` as a PNG file. The `.png` extension is appended automatically if not already present.

---

### `free_image`

```c
void free_image(image im);
```

Frees the memory allocated for `im`. Always call this when you are done with an image to avoid memory leaks.

---

## Pixel-Level Access and Utility Functions

### `get_pixel`

```c
float get_pixel(image im, int x, int y, int c);
```

Returns the pixel value at row `x`, column `y`, and channel `c`.

---

### `set_pixel`

```c
void set_pixel(image im, int x, int y, int c, float v);
```

Sets the pixel at row `x`, column `y`, channel `c` to value `v`.

---

### `copy_image`

```c
image copy_image(image im);
```

Returns a deep copy of the image. The returned image has its own independent memory allocation.

---

### `same_image`

```c
int same_image(image a, image b);
```

Performs a pixel-wise comparison between images `a` and `b`. Returns `1` if they are identical, `0` otherwise.

---

### `sub_image` and `add_image`

```c
image sub_image(image a, image b);
image add_image(image a, image b);
```

Perform element-wise subtraction or addition of pixel values between two images. Both images must have the same dimensions and channel count. The result is returned as a new image.

---

### `shift_image`

```c
void shift_image(image im, int c, float v);
```

Shifts every pixel in channel `c` of the image by adding `v` to each value. Useful for brightness adjustment on individual color channels.

---

## Color Manipulation

### `rgb_to_grayscale`

```c
image rgb_to_grayscale(image im);
```

Converts an RGB image to a single-channel grayscale image and returns it.

**Example:**

| Original | Grayscale |
|----------|-----------|
| ![original](https://github.com/user-attachments/assets/9bafd282-47af-42da-9a20-7f22839dc62f) | ![grayscale](https://github.com/user-attachments/assets/ad3c5611-d54b-495d-a743-246a8ee86da1) |

---

### `rgb_to_hsv`

```c
void rgb_to_hsv(image im);
```

Converts the image in-place from the RGB color space to the HSV (Hue, Saturation, Value) color space. Useful for tasks that benefit from separating color from brightness, such as color-based segmentation.

---

### `hsv_to_rgb`

```c
void hsv_to_rgb(image im);
```

Converts the image in-place from HSV back to RGB. Use this after performing operations in the HSV color space before displaying or saving the image.

---

## Corner Detection and Panorama

### Harris Corner Detector

She-sees uses the **Harris Corner Detector** to identify corner-like features in an image. Corners are regions where intensity changes significantly in multiple directions, making them useful as reliable keypoints for matching and stitching.

---

#### `detect_and_draw_corners`

```c
image detect_and_draw_corners(image im, float sigma, float thresh, int nms);
```

Detects corners in the image and draws them on a copy of the image. Returns the annotated image.

**Parameters:**

| Parameter | Description | Typical Value |
|-----------|-------------|---------------|
| `im` | Input image | — |
| `sigma` | Standard deviation for the Gaussian used in Harris | `2` |
| `thresh` | Threshold for the cornerness response | `1` – `5` |
| `nms` | Window size for non-maximum suppression on the response map | `3` |

**Example:**

| Detected Corners |
|-----------------|
| ![corners](https://github.com/user-attachments/assets/4599fa92-924d-45a2-932a-d4660dfdb097) |


---

#### `harris_corner_detector`

```c
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n);
```

Same detection logic as above, but returns detected corners in numerical form as an array of `descriptor` structs. The number of detected corners is written to `*n`.

```c
typedef struct {
    point p;   // position of the corner
    int n;     // descriptor length
    float *data; // descriptor data
} descriptor;
```

Use this when you need to work programmatically with corner locations and their descriptors rather than just visualizing them.

---

### Panorama Stitching

#### `panorama_image`

```c
image panorama_image(image a, image b, int save_intermediate,
                     float sigma, float thresh, int nms,
                     float inlier_thresh, int iters, int cutoff);
```

Stitches two images together into a panorama. Internally, it detects corners using the Harris detector, matches descriptors between the two images, and uses RANSAC to robustly estimate the homography for alignment.

**Parameters:**

| Parameter | Description | Typical Value |
|-----------|-------------|---------------|
| `a`, `b` | Images to stitch together | — |
| `save_intermediate` | If `1`, saves an intermediate image visualizing the matched keypoints and correspondences | `0` or `1` |
| `sigma` | Gaussian sigma for Harris corner detection | `2` |
| `thresh` | Cornerness threshold | `1` – `5` |
| `nms` | NMS window size | `3` |
| `inlier_thresh` | RANSAC inlier distance threshold | `2` – `5` |
| `iters` | Number of RANSAC iterations | `1,000` – `50,000` |
| `cutoff` | Minimum RANSAC inlier count to accept a homography | `10` – `100` |

**Example:**

| Image A | Image B |
|---------|---------|
| ![img_a](https://github.com/user-attachments/assets/9f25ecf6-e94a-47a5-adc5-781be57f632b) | ![img_b](https://github.com/user-attachments/assets/f094aa3b-15f9-40bc-bf2d-b64df0b2e89b) |



| Panorama Output | Intermediate (Matched Keypoints) |
|-----------------|----------------------------------|
| ![panorama](https://github.com/user-attachments/assets/6b1e08dc-cc9c-456f-89e9-a0b9af5bac8e) | ![intermediate](https://github.com/user-attachments/assets/bb1dddd2-6c45-41d3-a1c1-12015bdab5ec) |

---

## Optical Flow

Optical flow estimates the apparent motion of pixels between two consecutive frames. She-sees computes optical flow using the **Lucas-Kanade** method, which solves for per-pixel velocity using local gradient information.

---

### `optical_flow_images`

```c
image optical_flow_images(image im, image prev, int smooth, int stride);
```

Calculates the optical flow (velocity field) between two images.

**Parameters:**

| Parameter | Description |
|-----------|-------------|
| `im` | Current image |
| `prev` | Previous image in the sequence |
| `smooth` | Amount to smooth the structure matrix by |
| `stride` | Downsampling factor for the velocity matrix |

**Returns:** A velocity matrix image encoding the estimated motion field.

---

### `optical_flow_webcam`

```c
void optical_flow_webcam(int smooth, int stride, int div);
```

Runs a live optical flow demo using the webcam. This requires OpenCV and a webcam. Here's a demo.

[optical_flow_demo.webm](https://github.com/user-attachments/assets/8a4ba3bf-0910-4599-8b6a-6fc0d55db632)


**Parameters:**

| Parameter | Description | Recommended |
|-----------|-------------|-------------|
| `smooth` | Structure matrix smoothing | `15` |
| `stride` | Velocity sampling stride | `4` |
| `div` | Downsampling factor | `8` |

---

### Low-Level Operations

These functions expose the internal steps of the optical flow pipeline for cases where more control is needed.

---

#### `time_structure_matrix`

```c
image time_structure_matrix(image im, image prev, int s);
```

Computes the time-structure matrix from two consecutive images. This encodes spatial and temporal gradient information needed for optical flow estimation.

**Parameters:**

| Parameter | Description |
|-----------|-------------|
| `im` | Current image |
| `prev` | Previous image |
| `s` | Window size for smoothing |

**Returns:** A 5-channel structure matrix image with the following channels:

| Channel | Content |
|---------|---------|
| 1 | Ix² |
| 2 | Iy² |
| 3 | IxIy |
| 4 | IxIt |
| 5 | IyIt |

Here, `I` refers to the image gradient (not intensity).

---

#### `velocity_image`

```c
image velocity_image(image S, int stride);
```

Computes the velocity (motion vector) field from a smoothed time-structure matrix.

**Parameters:**

| Parameter | Description |
|-----------|-------------|
| `S` | Smoothed time-structure matrix (output of `time_structure_matrix`) |
| `stride` | Motion vectors are computed every `stride` pixels rather than at every pixel |

---

## Resizing

Both resize functions internally use their respective interpolation functions to predict pixel values at fractional positions in the source image.

### Nearest Neighbor Resize

#### `nn_interpolate`

```c
float nn_interpolate(image im, float x, float y, int c);
```

Given a floating-point position `(x, y)` and channel `c`, returns the pixel value of the nearest pixel in the source image.

---

#### `nn_resize`

```c
image nn_resize(image im, int w, int h, int use_box_filter);
```

Resizes the image to `w × h` using nearest neighbor interpolation. If `use_box_filter` is set to `1`, a box filter is applied before downsampling to reduce aliasing.

**Example:**

| Original | Resized |
|----------|---------------------------|
| ![original](https://github.com/user-attachments/assets/84315671-c76c-4270-9890-6afae50546c0) | ![nn_resized](https://github.com/user-attachments/assets/9f950fcd-6595-4ec6-81fd-ded1b7ff9fb3) |



---

### Bilinear Resize

#### `bilinear_interpolate`

```c
float bilinear_interpolate(image im, float x, float y, int c);
```

Given a floating-point position `(x, y)` and channel `c`, returns an interpolated pixel value using the weighted average of the four neighboring pixels.

---

#### `bilinear_resize`

```c
image bilinear_resize(image im, int w, int h, int use_box_filter);
```

Resizes the image to `w × h` using bilinear interpolation. Produces smoother results than nearest neighbor, especially when downscaling. If `use_box_filter` is `1`, a box filter is applied before downsampling.



## Filtering

### Convolution

#### `convolve_image`

```c
image convolve_image(image im, image filter, int preserve);
```

Applies a filter (kernel) to the image via convolution. The filter is applied channel-wise.

- If `preserve` is `1`, the output retains the original number of channels.
- If `preserve` is `0`, the channels are summed into a single-channel result.

---

### Available Filters

#### `make_box_filter`

```c
image make_box_filter(int w);
```

Creates a `w × w` box (mean) filter where all values are equal and sum to 1. Used for simple blurring and smoothing. Often used as a pre-step before downsampling to reduce aliasing.

---

#### `make_highpass_filter`

```c
image make_highpass_filter();
```

Creates a highpass filter kernel. Highpass filtering emphasizes rapid intensity changes, effectively removing the low-frequency (smooth) content from an image and retaining edges and fine details.

**Example:**

| Original | After Highpass Filter |
|----------|-----------------------|
| ![original](docs/images/dog.jpg) | ![highpass](docs/images/dog_highpass.jpg) |



---

#### `make_sharpen_filter`

```c
image make_sharpen_filter();
```

Creates a sharpening filter. It enhances edges and fine details by emphasizing high-frequency components while preserving the overall appearance of the image. Useful when an image appears soft or blurry.

---

#### `make_emboss_filter`

```c
image make_emboss_filter();
```

Creates an emboss filter that gives the image a raised, three-dimensional texture effect by highlighting edges in a single direction. The output simulates light falling across a surface.

**Example:**

| Original | After Emboss Filter |
|----------|---------------------|
| ![original](docs/images/dog.jpg) | ![emboss](docs/images/dog_emboss.jpg) |

---

#### `make_gaussian_filter`

```c
image make_gaussian_filter(float sigma);
```

Creates a Gaussian blur kernel with the given standard deviation `sigma`. Gaussian filtering attenuates high-frequency content (fine detail, noise) and retains low-frequency content (broad color regions, smooth gradients). 

Visually, the blurred image looks similar to the original, but if it is subtracted from the original, only the high-frequency features (edges, textures) remain — a technique commonly used in unsharp masking and frequency analysis.

**Example:**

| Original | After Gaussian Filter |
|----------|-----------------------|
| ![original](docs/images/dog.jpg) | ![gaussian](docs/images/dog_gaussian.jpg) |

---

#### `make_gx_filter`

```c
image make_gx_filter();
```

Creates a horizontal Sobel kernel (Gx). When convolved with an image, it computes the image gradient in the horizontal direction, highlighting **vertical edges** (boundaries where intensity changes left to right).

**Example:**

| Original | After Gx Filter (Vertical Edges) |
|----------|-----------------------------------|
| ![original](docs/images/dog.jpg) | ![gx](docs/images/dog_gx.jpg) |

---

#### `make_gy_filter`

```c
image make_gy_filter();
```

Creates a vertical Sobel kernel (Gy). When convolved with an image, it computes the image gradient in the vertical direction, highlighting **horizontal edges** (boundaries where intensity changes top to bottom).

**Example:**

| Original | After Gy Filter (Horizontal Edges) |
|----------|-------------------------------------|
| ![original](docs/images/dog.jpg) | ![gy](docs/images/dog_gy.jpg) |

---

### Normalization

#### `feature_normalize`

```c
void feature_normalize(image im);
```

Normalizes each pixel value using min-max normalization across the entire image:

```
normalized = (x - min) / (max - min)
```

Scales all pixel values to the range [0, 1]. Useful before displaying images that contain values outside the normal display range (e.g., filter response images).

---

#### `l1_normalize`

```c
void l1_normalize(image im);
```

Normalizes each channel of the image by dividing every pixel by the sum of all pixel values in that channel. Ensures that all channel values sum to 1.

---

### Sobel Edge Detection

#### `sobel_image`

```c
image *sobel_image(image im);
```

Applies the full Sobel operator to the image using both `make_gx_filter` and `make_gy_filter`. Returns a pointer to an array of two images:

- **Index 0:** Gradient magnitude — the overall strength of edges at each pixel.
- **Index 1:** Gradient direction — the angle of the edge at each pixel (in radians).

**Example:**

| Gradient Magnitude | Gradient Direction |
|--------------------|--------------------|
| ![sobel_mag](docs/images/dog_sobel_mag.jpg) | ![sobel_dir](docs/images/dog_sobel_dir.jpg) |

---

#### `colorize_sobel`

```c
image colorize_sobel(image im);
```

Produces a colorized visualization of the Sobel output by encoding gradient direction as hue and gradient magnitude as saturation and value. Useful for visually inspecting edge orientations across the image.

---

### Image Smoothing

#### `smooth_image`

```c
image smooth_image(image im, float sigma, int use_1d_gauss);
```

Smooths the image using a Gaussian filter with standard deviation `sigma`. If `use_1d_gauss` is set to `1`, a separable 1D Gaussian is applied (first horizontally, then vertically), which is significantly faster than applying the full 2D kernel — especially for large sigma values.

---

## Credits and Contributing

Part of this codebase originates from [Joseph Redmon's](https://pjreddie.com/) assignments for the course *Ancient Secrets of Computer Vision*. The project is open source, and contributions are encouraged. Feel free to open issues or pull requests on the [repository](https://github.com/baelthebard42/She-Sees).
