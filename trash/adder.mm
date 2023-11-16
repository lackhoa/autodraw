#import <stdio.h>
#import <Metal/Metal.h>
#import <Cocoa/Cocoa.h>
#import "utils.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;
global_variable b32 global_running = true;

global_variable i32 arrayLength = 32;

void generateRandomData(id<MTLBuffer> buffer)
{
  r32 *data = (r32 *)buffer.contents;
  for (i32 i=0; i < arrayLength; i++)
  {
    data[i] = (r32)rand() / (r32)RAND_MAX;
  }
}

int main(int argc, const char *argv[])
{
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();

  id<MTLLibrary> defaultLibrary = [device newDefaultLibrary];
  assert(defaultLibrary != nil);

  id<MTLFunction> addFunction = [defaultLibrary newFunctionWithName:@"add_arrays"];
  assert(addFunction != nil);

  NSError *error = nil;
  id<MTLComputePipelineState> addFunctionPSO = [device newComputePipelineStateWithFunction:addFunction error:&error];
  id<MTLCommandQueue> commandQueue = [device newCommandQueue];
   

  id<MTLBuffer> bufferA = [device newBufferWithLength:arrayLength options:MTLResourceStorageModeShared];
  id<MTLBuffer> bufferB = [device newBufferWithLength:arrayLength options:MTLResourceStorageModeShared];
  id<MTLBuffer> bufferC = [device newBufferWithLength:arrayLength options:MTLResourceStorageModeShared];

  generateRandomData(bufferA);
  generateRandomData(bufferB);

  id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
  id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
  [computeEncoder setComputePipelineState:addFunctionPSO];
  [computeEncoder setBuffer:bufferA offset:0 atIndex:0];
  [computeEncoder setBuffer:bufferB offset:0 atIndex:1];
  [computeEncoder setBuffer:bufferC offset:0 atIndex:2];

  MTLSize    gridSize         = MTLSizeMake(arrayLength, 1, 1);
  NSUInteger threadGroupSize_ = minimum(addFunctionPSO.maxTotalThreadsPerThreadgroup, arrayLength);
  MTLSize    threadgroupSize  = MTLSizeMake(threadGroupSize_, 1, 1);

  [computeEncoder dispatchThreads:gridSize threadsPerThreadgroup:threadgroupSize];
  [computeEncoder endEncoding];

  [commandBuffer commit];
  [commandBuffer waitUntilCompleted];

  {
    r32 *a = (r32 *)bufferA.contents;
    r32 *b = (r32 *)bufferB.contents;
    r32 *result = (r32 *)bufferC.contents;

    for (i32 index=0; index < arrayLength; index++)
    {
      if (result[index] != (a[index] + b[index]))
      {
        printf("Compute ERROR: index=%d result=%g vs %g=a+b\n",
               index, result[index], a[index] + b[index]);
        assert(result[index] == (a[index] + b[index]));
      }
    }
    printf("Compute results as expected\n");
  }

  printf("objective-c autodraw finished!\n");
}
