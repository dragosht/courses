#include    <wb.h>

#define wbCheck(stmt) do {                                 \
        cudaError_t err = stmt;                            \
        if (err != cudaSuccess) {                          \
            wbLog(ERROR, "Failed to run stmt ", #stmt);    \
            return -1;                                     \
        }                                                  \
    } while(0)


#define MASK_WIDTH  5
#define MASK_RADIUS 2

//@@ INSERT CODE HERE
#define TILE_WIDTH 16
#define SHARED_SIZE TILE_WIDTH + 2 * MASK_RADIUS

__device__ float clamp(float x, float start, float end)
{
    return (min(max(x, start), end));
}


__global__ void convolute(float* in, float* out, int height, int width,
			  int channels, const float *M)
{
    __shared__ float Ns[SHARED_SIZE][SHARED_SIZE];

    const int corners[] = {-MASK_RADIUS, MASK_RADIUS};
    int bx = blockIdx.x;
    int by = blockIdx.y;
    int tx = threadIdx.x;
    int ty = threadIdx.y;

    int row = by * blockDim.y + ty;
    int col = bx * blockDim.x + tx;

    if (row >= height || col >= width) {
	return;
    }

    for (int k = 0; k < channels; k++) {
	    for (int i = 0; i < 2; i++) {
	    for (int j = 0; j < 2; j++) {
		int x = corners[i];
		int y = corners[j];
		if (row + y < 0 || row + y >= height ||
		    col + x < 0 || col + x >= width) {
		    Ns[ty + y + MASK_RADIUS][tx + x + MASK_RADIUS] = 0.;
		} else {
		    Ns[ty + y + MASK_RADIUS][tx + x + MASK_RADIUS] =
			in[((row + y)* width + (col + x)) * channels + k];
		}
	    }
	}

	__syncthreads();
	float accum = 0.0;
	for (int y = -MASK_RADIUS; y <= MASK_RADIUS; y++) {
	    for (int x = -MASK_RADIUS; x <= MASK_RADIUS; x++) {
		int yOffset = row + y;
		int xOffset = col + x;

		if (xOffset >= 0 && xOffset < width &&
		    yOffset >= 0 && yOffset < height) {
		    float value = Ns[ty + y + MASK_RADIUS][tx + x + MASK_RADIUS];
		    //float value = in[((row + y)* width + (col + x)) * channels + k];
		    float mask = M[(y + MASK_RADIUS) * MASK_WIDTH + x + MASK_RADIUS];
		    accum += value * mask;
		}

	    }
	}

	out[(row * width + col) * channels + k] = clamp(accum, 0., 1.);
	__syncthreads();
    }
}


int main(int argc, char* argv[]) {
    wbArg_t arg;
    int maskRows;
    int maskColumns;
    int imageChannels;
    int imageWidth;
    int imageHeight;
    char * inputImageFile;
    char * inputMaskFile;
    wbImage_t inputImage;
    wbImage_t outputImage;
    float * hostInputImageData;
    float * hostOutputImageData;
    float * hostMaskData;
    float * deviceInputImageData;
    float * deviceOutputImageData;
    float * deviceMaskData;

    arg = wbArg_read(argc, argv); /* parse the input arguments */

    inputImageFile = wbArg_getInputFile(arg, 0);
    inputMaskFile = wbArg_getInputFile(arg, 1);

    inputImage = wbImport(inputImageFile);
    hostMaskData = (float *) wbImport(inputMaskFile, &maskRows, &maskColumns);

    assert(maskRows == 5); /* mask height is fixed to 5 in this mp */
    assert(maskColumns == 5); /* mask width is fixed to 5 in this mp */

    imageWidth = wbImage_getWidth(inputImage);
    imageHeight = wbImage_getHeight(inputImage);
    imageChannels = wbImage_getChannels(inputImage);

    outputImage = wbImage_new(imageWidth, imageHeight, imageChannels);

    hostInputImageData = wbImage_getData(inputImage);
    hostOutputImageData = wbImage_getData(outputImage);

    wbTime_start(GPU, "Doing GPU Computation (memory + compute)");

    wbTime_start(GPU, "Doing GPU memory allocation");
    cudaMalloc((void **) &deviceInputImageData, imageWidth * imageHeight * imageChannels * sizeof(float));
    cudaMalloc((void **) &deviceOutputImageData, imageWidth * imageHeight * imageChannels * sizeof(float));
    cudaMalloc((void **) &deviceMaskData, maskRows * maskColumns * sizeof(float));
    wbTime_stop(GPU, "Doing GPU memory allocation");


    wbTime_start(Copy, "Copying data to the GPU");
    cudaMemcpy(deviceInputImageData,
               hostInputImageData,
               imageWidth * imageHeight * imageChannels * sizeof(float),
               cudaMemcpyHostToDevice);
    cudaMemcpy(deviceMaskData,
               hostMaskData,
               maskRows * maskColumns * sizeof(float),
               cudaMemcpyHostToDevice);
    wbTime_stop(Copy, "Copying data to the GPU");


    wbTime_start(Compute, "Doing the computation on the GPU");
    //@@ INSERT CODE HERE
    dim3 dimBlock(TILE_WIDTH, TILE_WIDTH, 1);
    dim3 dimGrid((wbImage_getWidth(inputImage) - 1) / TILE_WIDTH + 1,
		 (wbImage_getHeight(inputImage) - 1) / TILE_WIDTH + 1,
		 1);
    convolute<<<dimGrid, dimBlock>>>(deviceInputImageData, deviceOutputImageData, imageHeight, imageWidth,
		imageChannels, deviceMaskData);

    wbTime_stop(Compute, "Doing the computation on the GPU");


    wbTime_start(Copy, "Copying data from the GPU");
    cudaMemcpy(hostOutputImageData,
               deviceOutputImageData,
               imageWidth * imageHeight * imageChannels * sizeof(float),
               cudaMemcpyDeviceToHost);
    wbTime_stop(Copy, "Copying data from the GPU");

    wbTime_stop(GPU, "Doing GPU Computation (memory + compute)");

    wbSolution(arg, outputImage);

    cudaFree(deviceInputImageData);
    cudaFree(deviceOutputImageData);
    cudaFree(deviceMaskData);

    free(hostMaskData);
    wbImage_delete(outputImage);
    wbImage_delete(inputImage);

    return 0;
}
