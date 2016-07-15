#pragma once
#include "app/Graphics/gfxApi.hpp"
#include "core/src/math/vec_templated.hpp"

namespace rendering
{
  namespace utils
  {
    class Graph
    {
    private:

      struct consts // reflects hlsl
      {
        faze::vec2 topleft;
        faze::vec2 bottomright;
        float val;
        float valueMin;
        float valueMax;
        int startUvX;
        int width;
        int height;
		    unsigned texIndex;
        float empty;
      };

      ComputePipeline m_cmdPipeline;
      GraphicsPipeline m_drawPipeline;
      Buffer m_uploadConstants;
      Buffer m_graphConstants;
      BufferCBV m_graphConstantsCbv;
	    Texture m_graphTexture;
      TextureUAV m_graphTextureUav;
      TextureSRV m_graphTextureSrv;

      faze::vec2 m_topleft;
      faze::vec2 m_bottomright;
      int currentUvX;
      int width;
      int height;
      float valueMin;
      float valueMax;
    public:
      Graph(GpuDevice& device, float min, float max, faze::ivec2 graphSize);
      void updateGraphCompute(GfxCommandList& gfx, float val);
      void drawGraph(GfxCommandList& gfx);
      void changeScreenPos(faze::vec2 topleft, faze::vec2 bottomright);
      void changeMin(float min);
      void changeMax(float max);
      void changeGraphResolution(GpuDevice& device, faze::ivec2 graphSize);
    };
  };
};
