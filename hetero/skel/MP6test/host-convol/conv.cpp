#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

#include <wb.h>

using namespace std;


float min(float a, float b)
{
    return ((a < b) ? a : b);
}

float max(float a, float b)
{
    return ((a > b) ? a : b);
}

float clamp(float x, float start, float end)
{
    return (min(max(x, start), end));
}


int main(int argc, char *argv[])
{
    wbArg_t arg;
    int maskRows;
    int maskColumns;
    int imageChannels;
    int imageWidth;
    int imageHeight;
    char* inputImageFile;
    char* inputMaskFile;
    wbImage_t inputImage;
    wbImage_t outputImage;
    float* inputImageData;
    float* outputImageData;
    float* maskData;

    arg = wbArg_read(argc, argv); /* parse the input arguments */

    inputImageFile = wbArg_getInputFile(arg, 0);
    inputMaskFile = wbArg_getInputFile(arg, 1);

    inputImage = wbImport(inputImageFile);
    maskData = (float *) wbImport(inputMaskFile, &maskRows, &maskColumns);

    assert(maskRows == 5); /* mask height is fixed to 5 in this mp */
    assert(maskColumns == 5); /* mask width is fixed to 5 in this mp */


    imageWidth = wbImage_getWidth(inputImage);
    imageHeight = wbImage_getHeight(inputImage);
    imageChannels = wbImage_getChannels(inputImage);

    outputImage = wbImage_new(imageWidth, imageHeight, imageChannels);

    inputImageData = wbImage_getData(inputImage);
    outputImageData = wbImage_getData(outputImage);

    int maskWidth = 5;
    int maskRadius = 2;

#ifdef CONV_DEBUG
    cout << endl << "Input: " << endl;

    for (int i = 0; i < imageHeight; i++) {
        for (int j = 0; j < imageHeight; j++) {
            cout << inputImageData[(i * imageWidth + j) * imageChannels + 0] << "/" <<
                inputImageData[(i * imageWidth + j) * imageChannels + 1] << "/" <<
                inputImageData[(i * imageWidth + j) * imageChannels + 2] << " ";
        }
        cout << endl;
    }
#endif //CONV_DEBUG

    for (int i = 0; i < imageHeight; i++) {
        for (int j = 0; j < imageWidth; j++) {
            for (int k = 0; k < imageChannels; k++) {
                float accum = 0.0;
                for (int y = -maskRadius; y <= maskRadius; y++) {
                    for (int x = -maskRadius; x <= maskRadius; x++) {
                        int xOffset = j + x;
                        int yOffset = i + y;
                        if (xOffset >= 0 && xOffset < imageWidth &&
                            yOffset >= 0 && yOffset < imageHeight) {
                            float pixel = inputImageData[(yOffset * imageWidth + xOffset) * imageChannels + k];
                            float mask = maskData[(y + maskRadius) * maskWidth + x + maskRadius];
                            accum += pixel * mask;
                        }
                    }
                }
                cout << "Setting pixel at: " << i << " " << j << " at: " << clamp(accum, 0., 1.) << " accum: " << accum << endl;
                outputImageData[(i * imageWidth + j) * imageChannels + k] = clamp(accum, 0., 1.);
            }
        }
    }

#ifdef CONV_DEBUG
    cout << endl << "Output" << endl;

    for (int i = 0; i < imageHeight; i++) {
        for (int j = 0; j < imageHeight; j++) {
            cout << outputImageData[(i * imageWidth + j) * imageChannels + 0] << "/" <<
                outputImageData[(i * imageWidth + j) * imageChannels + 1] << "/" <<
                outputImageData[(i * imageWidth + j) * imageChannels + 2] << " ";
        }
        cout << endl;
    }
#endif //CONV_DEBUG

    wbSolution(arg, outputImage);

    return 0;
}


