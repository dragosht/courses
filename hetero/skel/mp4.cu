// MP 4 Reduction
// Given a list (lst) of length n
// Output its sum = lst[0] + lst[1] + ... + lst[n-1];

#include    <wb.h>

#define BLOCK_SIZE 512 //@@ You can change this

#ifndef wbCheck
#define wbCheck(stmt) do {                                 \
        cudaError_t err = stmt;                            \
        if (err != cudaSuccess) {                          \
            wbLog(ERROR, "Failed to run stmt ", #stmt);    \
            return -1;                                     \
        }                                                  \
} while(0)
#endif //wbCheck


__global__ void total(float * input, float * output, int len) {
    //@@ Load a segment of the input vector into shared memory
    //@@ Traverse the reduction tree
    //@@ Write the computed sum of the block to the output vector at the
    //@@ correct index
    __shared__ float partSum[2 * BLOCK_SIZE];
    unsigned int b = blockIdx.x;
    unsigned int t = threadIdx.x;
    unsigned int start = 2 * blockIdx.x * blockDim.x;

    if (start + t < len) {
        partSum[t] = input[start + t];
    } else {
	partSum[t] = 0.0;
    }

    if (start + blockDim.x + t < len) {
        partSum[blockDim.x + t] = input[start + blockDim.x + t];
    } else {
	partSum[blockDim.x + t] = 0.0;
    }

    //The inefficient version
    /*
    for (unsigned int stride = 1; stride <= blockDim.x; stride *= 2) {
	__syncthreads();
	if (t % stride == 0) {
	    partSum[2 * t] += partSum[2 * t + stride];
	}
    }
    */

    //The better version
    for (unsigned int stride = blockDim.x; stride > 0; stride /= 2) {
	__syncthreads();
	if (t < stride) {
	    partSum[t] += partSum[t + stride];
	}
    }

    if (t == 0) {
	output[b] = partSum[0];
    }
}

int main(int argc, char ** argv) {
    int ii;
    wbArg_t args;
    float * hostInput; // The input 1D list
    float * hostOutput; // The output list
    float * deviceInput;
    float * deviceOutput;
    int numInputElements; // number of elements in the input list
    int numOutputElements; // number of elements in the output list

    args = wbArg_read(argc, argv);

    wbTime_start(Generic, "Importing data and creating memory on host");
    hostInput = (float *) wbImport(wbArg_getInputFile(args, 0), &numInputElements);

    numOutputElements = numInputElements / (BLOCK_SIZE << 1);
    if (numInputElements % (BLOCK_SIZE<<1)) {
        numOutputElements++;
    }
    hostOutput = (float*) malloc(numOutputElements * sizeof(float));

    wbTime_stop(Generic, "Importing data and creating memory on host");

    wbLog(TRACE, "The number of input elements in the input is ", numInputElements);
    wbLog(TRACE, "The number of output elements in the input is ", numOutputElements);

    wbTime_start(GPU, "Allocating GPU memory.");
    //@@ Allocate GPU memory here
    wbCheck(cudaMalloc((void**) &deviceInput, numInputElements * sizeof(float)));
    wbCheck(cudaMalloc((void**) &deviceOutput, numOutputElements * sizeof(float)));

    wbTime_stop(GPU, "Allocating GPU memory.");

    wbTime_start(GPU, "Copying input memory to the GPU.");
    //@@ Copy memory to the GPU here
    wbCheck(cudaMemcpy(deviceInput, hostInput, numInputElements * sizeof(float), cudaMemcpyHostToDevice));

    wbTime_stop(GPU, "Copying input memory to the GPU.");
    //@@ Initialize the grid and block dimensions here

    wbTime_start(Compute, "Performing CUDA computation");
    //@@ Launch the GPU Kernel here
    dim3 dimGrid((numInputElements - 1) / BLOCK_SIZE + 1, 1, 1);
    dim3 dimBlock(BLOCK_SIZE, 1, 1);

    for (ii = 0; ii < numOutputElements; ii++) {
	total<<<dimGrid, dimBlock>>>(deviceInput, deviceOutput, numInputElements);
    }

    wbCheck(cudaDeviceSynchronize());
    wbTime_stop(Compute, "Performing CUDA computation");

    wbTime_start(Copy, "Copying output memory to the CPU");
    //@@ Copy the GPU memory back to the CPU here
    wbCheck(cudaMemcpy(hostOutput, deviceOutput, numOutputElements * sizeof(float), cudaMemcpyDeviceToHost));

    wbTime_stop(Copy, "Copying output memory to the CPU");

    /********************************************************************
     * Reduce output vector on the host
     * NOTE: One could also perform the reduction of the output vector
     * recursively and support any size input. For simplicity, we do not
     * require that for this lab.
     ********************************************************************/
    for (ii = 1; ii < numOutputElements; ii++) {
        hostOutput[0] += hostOutput[ii];
    }

    wbTime_start(GPU, "Freeing GPU Memory");
    //@@ Free the GPU memory here
    wbCheck(cudaFree(deviceInput));
    wbCheck(cudaFree(deviceOutput));

    wbTime_stop(GPU, "Freeing GPU Memory");

    wbSolution(args, hostOutput, 1);

    free(hostInput);
    free(hostOutput);

    return 0;
}

