#include <opencv2/opencv.hpp>
int main()
{
    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        printf("Webcam not accessible\n");
        return -1;
    }
    cv::Mat frame;
    while (true)
    {
        cap >> frame;
        if (frame.empty())
            break;
        cv::imshow("Webcam Test", frame);
        if (cv::waitKey(30) >= 0)
            break;
    }
    return 0;
}
