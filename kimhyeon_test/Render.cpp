#include "Render.h"
#include <thread>
#include <random>

LRESULT CALLBACK msgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND createWindow(const char* winTitle, UINT width, UINT height);

void Render::run(content ctx) {

  Texture texture{&srvHeap, &cmdqueue, DXGI_FORMAT_R8G8B8A8_UNORM,
                  ctx.texture_path.c_str()};

  uint2 texture_size = uint2(texture.getWidth(), texture.getHeight());
  uint2 tile_size = ctx.tile_size;
  uint2 virtual_texture_size = ctx.virtual_texture_size;
  uint2 result_texture_size = ctx.out_texture_szie;

  RenderTarget textureTarget{&srvHeap,
                             &rtvHeap,
                             &cmdqueue,
                             DXGI_FORMAT_R32G32B32A32_FLOAT,
                             result_texture_size.x,
                             result_texture_size.y};

  UINT size = result_texture_size.x * result_texture_size.y * 4;
  std::vector<float> randomData(size, 0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0, 1);

  for (int i = 0; i < size; i += 4) {
    randomData[i] = dis(gen);
    randomData[i + 1] = randomData[i];
    randomData[i + 2] = randomData[i];
    randomData[i + 3] = randomData[i];
  }

  Texture randomTexture{&srvHeap,
                        &cmdqueue,
                        DXGI_FORMAT_R32G32B32A32_FLOAT,
                        result_texture_size.x,
                        result_texture_size.y,
                        randomData.data()};

  texturePass.bind("indirectTexture", texture.getSrv());
  texturePass.bind("randomTexture", randomTexture.getSrv());
  texturePass.bindRenderTarget(textureTarget);
  texturePass.setTargetSize(result_texture_size.x, result_texture_size.y);
  texturePass.bind("data", {texture_size, tile_size, virtual_texture_size});
  texturePass.render(&cmdqueue, &cmdlist);

  DXGI_FORMAT format = textureTarget.getFormat();
  UINT outW = textureTarget.getWidth();
  UINT outH = textureTarget.getHeight();
  ReadbackBuffer readbackBuff(_bpp(format) * outW * outH);
  readbackBuff.readback(cmdlist.begin(), textureTarget);
  cmdlist.end(&cmdqueue);

  void writeImageAsJpg(const char* filePath, const float* src, UINT width,
                       UINT height, UINT channels);

  writeImageAsJpg(ctx.out_path.c_str(), (float*)readbackBuff.map(), outW, outH,
                  _channels(format));
}

LRESULT CALLBACK msgProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
    case WM_SIZE:

      return 0;
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND createWindow(const char* winTitle, UINT width, UINT height) {
  WNDCLASSA wc = {};
  wc.lpfnWndProc = msgProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = "anything";
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClassA(&wc);

  RECT r{0, 0, (LONG)width, (LONG)height};
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);

  HWND hWnd =
      CreateWindowA(wc.lpszClassName, winTitle, WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left,
                    r.bottom - r.top, nullptr, nullptr, wc.hInstance, nullptr);

  return hWnd;
}