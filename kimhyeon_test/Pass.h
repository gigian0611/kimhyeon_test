#include "Helper.h"

struct PassLayout {
  struct Layout {
    inline static const D3D12_INPUT_LAYOUT_DESC inputLayout = {};
  };
  struct Sampler {
    inline static const std::vector<D3D12_STATIC_SAMPLER_DESC> descArr = {};
  };
  struct RenderTarget {
    inline static const std::vector<DXGI_FORMAT> format = {DXGI_FORMAT_UNKNOWN};

    inline static const std::vector<BlendMode> blendMode = {BlendMode::blend_opaque};

    inline static const UINT count = format.size();
  };
  struct Draw {
    static void draw(ID3D12GraphicsCommandList* cmdList) {}
  };
  struct DepthTarget {
    static const DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    static const DepthMode depthMode = DepthMode::depth_disable;
  };
};

template <typename PassDesc>
class Pass {
  std::string vsEntry = "VSMain";
  std::string psEntry = "PSMain";

  DescriptorHeap* srvHeap;
  GraphicsPipeline pipeline{srvHeap};
  RootSignature* rootSigature = PassDesc::createRootSignature();

 public:
  explicit Pass(DescriptorHeap* _srvHeap) : srvHeap(_srvHeap) {
    rootSigature->build(
        PassDesc::Sampler::descArr,
        PassDesc::Layout::inputLayout.NumElements > 0
            ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            : D3D12_ROOT_SIGNATURE_FLAG_NONE);

    std::string srcPath = PassDesc::hlslName;

    pipeline.build(
        rootSigature, PassDesc::Layout::inputLayout,
        dxShader(srcPath.c_str(), vsEntry.c_str(), "vs_5_0").getCode(),
        dxShader(srcPath.c_str(), psEntry.c_str(), "ps_5_0").getCode(),
        PassDesc::RenderTarget::count, PassDesc::RenderTarget::blendMode,
        PassDesc::DepthTarget::depthMode, PassDesc::RenderTarget::format,
        PassDesc::DepthTarget::format, D3D12_CULL_MODE_NONE);

#ifdef _DEBUG
    wchar_t* wname = new wchar_t[strlen(srcPath.c_str()) + 1];
    mbstowcs(wname, srcPath.c_str(), strlen(srcPath.c_str()) + 1);
    pipeline.SetName(wname);
    delete[] wname;
#endif
  }

  ~Pass() { delete rootSigature; }

  void bind(std::string_view name,
            const typename PassDesc::ConstantData& data) {
    rootSigature->set(name, data);
  }

  template <typename T>
  void bind(std::string_view name, const T& data) {
    rootSigature->set(name, data);
  }

  void bindRenderTarget(const RenderTarget& rt, UINT rtIdx = 0) {
    bindRenderTarget(rt.getRtv(), rtIdx);
  }

  void bindDepthTarget(const DepthTarget& dt) { bindDepthTarget(dt.getDsv()); }

  void bindRenderTarget(const Descriptor& rtv, UINT rtIdx = 0) {
    pipeline.setRtHandle(rtIdx, rtv);
  }

  void bindDepthTarget(const Descriptor& dsv) { pipeline.setDtHandle(dsv); }

  void setTargetSize(UINT width, UINT height) {
    pipeline.setViewport({0.0f, 0.0f, static_cast<float>(width),
                          static_cast<float>(height), 0.0f, 1.0f});
    pipeline.setScissorRect({(LONG)0, (LONG)0, (LONG)width, (LONG)height});
  }

  void clearTargetsBeforeNextRender() {
    pipeline.clearTargetsBeforeNextRender();
  }

  template <typename... Params>
  void render(CommandQueue* queue, CommandList* cmdList, Params... params) {
    ID3D12GraphicsCommandList* rawList = cmdList->begin();
    {
      pipeline.begin(rawList);
      PassDesc::Draw::draw(rawList, params...);
      pipeline.end();
    }
    cmdList->end(queue);
  }
};

struct TestSpace : PassLayout {
  inline static const char* hlslName = "./TestSpacePass.hlsl";

  struct RenderTarget {
    inline static const std::vector<DXGI_FORMAT> format = {
        DXGI_FORMAT_R32G32B32A32_FLOAT};

    inline static const std::vector<BlendMode> blendMode = {
        BlendMode::blend_opaque};

    inline static const UINT count = format.size();
  };

  struct RenderInfo {
    D3D12_VERTEX_BUFFER_VIEW vtxBuffView{};
    D3D12_INDEX_BUFFER_VIEW idxBuffView{};
    UINT numTriangles{};
  };

  struct Draw {
    static void draw(ID3D12GraphicsCommandList* cmdList) {
      cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
      cmdList->DrawInstanced(4, 1, 0, 0);
    }
  };

  struct Sampler {
    inline static const std::vector<D3D12_STATIC_SAMPLER_DESC> descArr = {
        D3D12_STATIC_SAMPLER_DESC{
            .Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .ShaderRegister = 0,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL}};
  };

  struct ConstantData {
    uint2 texture_size;
    uint2 tile_size;
    uint2 virtual_texture_size;
    uint2 pad;
  };

  static RootSignature* createRootSignature() {
    return new RootSignature{{"data", RootConstants("b0", ConstantData{})},
                             {"indirectTexture", RootTable("t0")},
                             {"randomTexture", RootTable("t1")}};
  }
};
