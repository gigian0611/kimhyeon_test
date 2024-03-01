
#include "Render.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


#ifdef _DEBUG
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


int main() {

  Render::content ctx;
  ctx.texture_path = "./data/texture.png";
  ctx.out_path = "./out/result.png";
  ctx.tile_size = uint2(128, 128);
  ctx.virtual_texture_size = uint2(65536, 65536);
  ctx.out_texture_szie = uint2(512, 512);

  std::unique_ptr<Render> render = std::make_unique<Render>();
  render->run(ctx);
 
  return 0;
}

