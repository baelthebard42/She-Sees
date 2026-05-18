#define OPENCV
#include "image.h"

int main()
{
    // Load original image
    image original = load_image("./data/sagarmatha.jpg");

    image corners_drawn = detect_and_draw_corners(original, 2, 50, 3 );
    save_image(corners_drawn, "sagarmatha_with_corners");

    image sa = load_image("./data/sagarmatha_a.png");
    image sb = load_image("./data/sagarmatha_b.png");

    image pano = panorama_image(sa, sb, 2, 5, 3, 3, 50000, 90);

    save_image(pano, "sagarmatha_panoroma");






}