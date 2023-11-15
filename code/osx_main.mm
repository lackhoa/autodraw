#include <stdio.h>

// #include <AppKit/AppKit.h>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "utils.h"

global_variable r32 global_rendering_width = 1024;
global_variable r32 global_rendering_height = 768;
global_variable b32 global_running = true;

global_variable i32 arrayLength = 32;

void generateRandomData(MTL::Buffer *buffer)
{
  r32 *data = (r32 *)buffer->contents();
  for (i32 i=0; i < arrayLength; i++)
  {
    data[i] = (r32)rand() / (r32)RAND_MAX;
  }
}

int main(int argc, const char *argv[])
{
  MTL::Device *device = MTL::CreateSystemDefaultDevice();

  MTL::Library *defaultLibrary = device->newDefaultLibrary();
  assert(defaultLibrary);

  MTL::Function *addFunction = defaultLibrary->newFunction((const NS::String *)@"add_arrays");
  assert(addFunction);

  NS::Error* error = nil;
  MTL::ComputePipelineState *addFunctionPSO = device->newComputePipelineState(addFunction, &error);
  MTL::CommandQueue *commandQueue = device->newCommandQueue();

  MTL::Buffer *bufferA = device->newBuffer(arrayLength, MTL::ResourceStorageModeShared);
  MTL::Buffer *bufferB = device->newBuffer(arrayLength, MTL::ResourceStorageModeShared);
  MTL::Buffer *bufferResult = device->newBuffer(arrayLength, MTL::ResourceStorageModeShared);

  generateRandomData(bufferA);
  generateRandomData(bufferB);

  MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
  MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
  computeEncoder->setComputePipelineState(addFunctionPSO);
  computeEncoder->setBuffer(bufferA,0,0);
  computeEncoder->setBuffer(bufferB,0,1);
  computeEncoder->setBuffer(bufferResult,0,2);

  MTL::Size    gridSize         = MTL::Size::Make(arrayLength, 1, 1);
  NS::UInteger threadGroupSize_ = minimum(addFunctionPSO->maxTotalThreadsPerThreadgroup(), arrayLength);
  MTL::Size    threadgroupSize  = MTL::Size::Make(threadGroupSize_, 1, 1);

  computeEncoder->dispatchThreads(gridSize, threadgroupSize);
  computeEncoder->endEncoding();

  commandBuffer->commit();
  commandBuffer->waitUntilCompleted();

  {
    r32 *a = (r32 *)bufferA->contents();
    r32 *b = (r32 *)bufferB->contents();
    r32 *result = (r32 *)bufferResult->contents();

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

  printf("autodraw finished!\n");
}
