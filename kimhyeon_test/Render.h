#pragma once
#include "Helper.h"
#include "Input.h"
#include "Camera.h"
#include "Pass.h"
#include <thread>


class Render {
 private:
  DescriptorHeap rtvHeap{128, D3D12_DESCRIPTOR_HEAP_TYPE_RTV};
  DescriptorHeap srvHeap{256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV};
  DescriptorHeap dsvHeap{32, D3D12_DESCRIPTOR_HEAP_TYPE_DSV};
  CommandQueue cmdqueue{D3D12_COMMAND_LIST_TYPE_DIRECT};
  CommandList cmdlist{D3D12_COMMAND_LIST_TYPE_DIRECT};

  HWND hwnd = nullptr;
  UINT renderWidth = 1200;
  UINT renderHeight = 900;
  SwapChain swapChain;

  Pass<TestSpace> texturePass{&srvHeap};

 public:
  struct content {
    std::string texture_path = "";
    std::string out_path = "";
    uint2 tile_size;
    uint2 virtual_texture_size;
    uint2 out_texture_szie;
  };
  void run(content ctx);
};
