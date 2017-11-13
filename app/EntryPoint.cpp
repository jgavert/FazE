#include "core/src/Platform/EntryPoint.hpp"

// EntryPoint.cpp
#ifdef FAZE_PLATFORM_WINDOWS
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include "core/src/neural/network.hpp"
#include "core/src/system/LBS.hpp"
#include "core/src/system/time.hpp"
#include "faze/src/new_gfx/GraphicsCore.hpp"
#include "core/src/filesystem/filesystem.hpp"
#include "core/src/Platform/Window.hpp"
#include "core/src/system/logger.hpp"
#include "core/src/Platform/EntryPoint.hpp"
#include "core/src/global_debug.hpp"

#include "core/src/math/vec_templated.hpp"

#include "shaders/textureTest.if.hpp"
#include "shaders/triangle.if.hpp"

#include "renderer/blitter.hpp"
#include "renderer/imgui_renderer.hpp"

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "core/src/external/imgiu/imgui.h"

#include "faze/src/new_gfx/common/cpuimage.hpp"
#include "faze/src/new_gfx/common/image_loaders.hpp"

using namespace faze;

class DynamicScale
{
private:
  int m_scale = 10;
  float m_frametimeTarget = 16.6667f; // 60fps
  float m_targetThreshold = 1.f; // 1 millisecond

public:
  DynamicScale(int scale = 10, float frametimeTarget = 16.6667f, float threshold = 1.f)
    : m_scale(scale)
    , m_frametimeTarget(frametimeTarget)
    , m_targetThreshold(threshold)
  {}

  int tick(float frametimeInMilli)
  {
    auto howFar = std::fabs((m_frametimeTarget - frametimeInMilli)*10.f);
    if (frametimeInMilli > m_frametimeTarget)
    {
      //F_LOG("Decreasing scale %d %f %f\n", m_scale, frametimeInMilli, m_frametimeTarget);
		m_scale -= int(howFar * 2);
    }
    else if (frametimeInMilli < m_frametimeTarget - m_targetThreshold)
    {
        m_scale += int(howFar);
    }
    if (m_scale <= 0)
      m_scale = 1;
    return m_scale;
  }
};

int EntryPoint::main()
{
  Logger log;

  auto main = [&](GraphicsApi api, VendorID preferredVendor, bool updateLog)
  {
    bool reInit = false;
    int64_t frame = 1;
    FileSystem fs;
    WTime time;

    auto image = textureUtils::loadImageFromFilesystem(fs, "/simple.jpg");
    log.update();

	bool explicitID = false;
    int chosenGpu = 0;

    while (true)
    {
      GraphicsSubsystem graphics(api, "faze");
      F_LOG("Using api %s\n", graphics.gfxApi().c_str());
      F_LOG("Have gpu's\n");
      auto gpus = graphics.availableGpus();
	  if (!explicitID)
	  {
		  for (auto&& it : gpus)
		  {
			  if (it.vendor == preferredVendor)
			  {
				  chosenGpu = it.id;
			  }
			  F_LOG("\t%d. %s (memory: %zd, api: %s)\n", it.id, it.name.c_str(), it.memory, it.apiVersionStr.c_str());
		  }
	  }
      if (updateLog) log.update();
      if (gpus.empty())
        return;

      ivec2 ires = { 200, 100 };

      int raymarch_Res = 10;
      int raymarch_x = 16;
      int raymarch_y = 9;

      float raymarchfpsTarget = 16.666667f;
      float raymarchFpsThr = 0.2f;
      bool raymarchDynamicEnabled = false;

      Window window(m_params, gpus[chosenGpu].name, 1280, 720, 300, 200);
      window.open();

      auto surface = graphics.createSurface(window);
      auto dev = graphics.createDevice(fs, gpus[chosenGpu]);
      time.firstTick();
      {
        auto toggleHDR = false;
        auto scdesc = SwapchainDescriptor().formatType(FormatType::Unorm8x4).colorspace(Colorspace::BT709).bufferCount(3);
        auto swapchain = dev.createSwapchain(surface, scdesc);

        F_LOG("Created device \"%s\"\n", gpus[chosenGpu].name.c_str());

        auto bufferdesc = ResourceDescriptor()
          .setName("testBufferTarget")
          .setFormat(FormatType::Float32)
          .setWidth(100)
          .setDimension(FormatDimension::Buffer);

        auto buffer = dev.createBuffer(bufferdesc);

        auto testImage = dev.createTexture(image);
        auto testSrv = dev.createTextureSRV(testImage, ShaderViewDescriptor().setMostDetailedMip(0).setMipLevels(1));

        auto texture = dev.createTexture(ResourceDescriptor()
          .setName("testTexture")
          .setFormat(FormatType::Unorm16x4)
          .setWidth(ires.x())
          .setHeight(ires.y())
          .setMiplevels(4)
          .setDimension(FormatDimension::Texture2D)
          .setUsage(ResourceUsage::RenderTargetRW));
        auto texRtv = dev.createTextureRTV(texture, ShaderViewDescriptor().setMostDetailedMip(0));
        auto texSrv = dev.createTextureSRV(texture, ShaderViewDescriptor().setMostDetailedMip(0).setMipLevels(1));
        auto texUav = dev.createTextureUAV(texture, ShaderViewDescriptor().setMostDetailedMip(0).setMipLevels(1));

        Renderpass triangleRenderpass = dev.createRenderpass();

        auto pipelineDescriptor = GraphicsPipelineDescriptor()
          .setVertexShader("triangle")
          .setPixelShader("triangle")
          .setPrimitiveTopology(PrimitiveTopology::Triangle)
          .setDepthStencil(DepthStencilDescriptor()
            .setDepthEnable(false));

        GraphicsPipeline trianglePipe = dev.createGraphicsPipeline(pipelineDescriptor);
        F_LOG("%d\n", trianglePipe.descriptor.sampleCount);

        renderer::Blitter blit(dev);
        renderer::ImGui imgRenderer(dev);

        ComputePipeline testCompute = dev.createComputePipeline(ComputePipelineDescriptor()
          .shader("textureTest"));

        bool closeAnyway = false;

        while (!window.simpleReadMessages(frame++))
        {
          fs.updateWatchedFiles();
          if (window.hasResized() || toggleHDR)
          {
            dev.adjustSwapchain(swapchain, scdesc);
            window.resizeHandled();
            toggleHDR = false;
          }
          auto& inputs = window.inputs();

          if (inputs.isPressedThisFrame(112, 1))
          {
            window.captureMouse(true);
          }
          if (inputs.isPressedThisFrame(113, 1))
          {
            window.captureMouse(false);
          }

          if (inputs.isPressedThisFrame(VK_MENU, 2) && inputs.isPressedThisFrame('1', 1))
          {
            window.toggleBorderlessFullscreen();
          }

          if (frame > 10 && (closeAnyway || inputs.isPressedThisFrame(VK_ESCAPE, 1)))
          {
            break;
          }
          if (updateLog) log.update();

          // If you acquire, you must submit it. Next, try to first present empty image.
          // On vulkan, need to at least clear the image or we will just get error about it. (... well at least when the contents are invalid in the beginning.)
          auto backbuffer = dev.acquirePresentableImage(swapchain);
          CommandGraph tasks = dev.createGraph();
          {
            auto node = tasks.createPass("clear");
            node.clearRT(backbuffer, vec4{ std::sin(time.getFTime())*.5f + .5f, 0.f, 0.f, 0.f });
            //node.clearRT(texRtv, vec4{ std::sin(float(frame)*0.01f)*.5f + .5f, std::sin(float(frame)*0.01f)*.5f + .5f, 0.f, 1.f });
            tasks.addPass(std::move(node));
          }

          {
            auto node = tasks.createPass("compute!");

            auto binding = node.bind<::shader::TextureTest>(testCompute);
            binding.constants = {};
            uint2 iRes = uint2{ texUav.desc().desc.width, texUav.desc().desc.height };
            binding.constants.iResolution = iRes;
            binding.constants.iFrame = static_cast<int>(frame);
            binding.constants.iTime = time.getFTime();
            binding.uav(::shader::TextureTest::output, texUav);

            unsigned x = static_cast<unsigned>(divideRoundUp(static_cast<uint64_t>(iRes.x()), 8));
            unsigned y = static_cast<unsigned>(divideRoundUp(static_cast<uint64_t>(iRes.y()), 8));
            node.dispatch(binding, uint3{ x, y, 1 });

            tasks.addPass(std::move(node));
          }
          //if (inputs.isPressedThisFrame('3', 2))

          {
            // we have pulsing red color background, draw a triangle on top of it !
            auto node = tasks.createPass("Triangle!");
            node.renderpass(triangleRenderpass);
            node.subpass(backbuffer);

            vector<float4> vertices;
            vertices.push_back(float4{ -1.f, -1.f, 1.f, 1.f });
            vertices.push_back(float4{ -1.0f, 3.f, 1.f, 1.f });
            vertices.push_back(float4{ 3.f, -1.f, 1.f, 1.f });

            auto verts = dev.dynamicBuffer(makeMemView(vertices), FormatType::Float32x4);

            auto binding = node.bind<::shader::Triangle>(trianglePipe);
            //binding.constants.color = float4{ 0.f, 0.f, std::sin(float(frame)*0.01f + 1.0f)*.5f + .5f, 1.f };
            binding.constants.color = float4{ 0.f, 0.f, 0.f, 1.f };
            binding.constants.colorspace = static_cast<int>(swapchain.impl()->displayCurve());
            binding.srv(::shader::Triangle::vertices, verts);
            binding.srv(::shader::Triangle::yellow, texSrv);
            node.draw(binding, 3, 1);
            node.endRenderpass();
            tasks.addPass(std::move(node));
          }
          //float heightMulti = 1.f; // float(testSrv.desc().desc.height) / float(testSrv.desc().desc.width);
          //blit.blitImage(dev, tasks, backbuffer, testSrv, renderer::Blitter::FitMode::Fit);

		  /*
          uint2 sdim = { testSrv.desc().desc.width, testSrv.desc().desc.height };
          float scale = float(sdim.x()) / float(sdim.y());

          float nwidth = float(backbuffer.desc().desc.width)*0.2f;
          float nheight = nwidth / scale;

          blit.blit(dev, tasks, backbuffer, testSrv, { 4, 800 }, int2{ int(nwidth), int(nheight) });
		  */

          {
            ImGuiIO &io = ImGui::GetIO();
            time.tick();
            io.DeltaTime = time.getFrameTimeDelta();
            if (io.DeltaTime < 0.f)
              io.DeltaTime = 0.00001f;

            auto& mouse = window.mouse();

            if (mouse.captured)
            {
              io.MousePos.x = static_cast<float>(mouse.m_pos.x());
              io.MousePos.y = static_cast<float>(mouse.m_pos.y());

              io.MouseWheel = static_cast<float>(mouse.mouseWheel)*0.1f;
            }

            auto& input = window.inputs();

            input.goThroughThisFrame([&](Input i)
            {
              if (i.key >= 0)
              {
                switch (i.key)
                {
                case VK_LBUTTON:
                {
                  io.MouseDown[0] = (i.action > 0);
                  break;
                }
                case VK_RBUTTON:
                {
                  io.MouseDown[1] = (i.action > 0);
                  break;
                }
                case VK_MBUTTON:
                {
                  io.MouseDown[2] = (i.action > 0);
                  break;
                }
                default:
                  if (i.key < 512)
                    io.KeysDown[i.key] = (i.action > 0);
                  break;
                }
              }
            });

            for (auto ch : window.charInputs())
              io.AddInputCharacter(static_cast<ImWchar>(ch));

            io.KeyCtrl = inputs.isPressedThisFrame(VK_CONTROL, 2);
            io.KeyShift = inputs.isPressedThisFrame(VK_SHIFT, 2);
            io.KeyAlt = inputs.isPressedThisFrame(VK_MENU, 2);
            io.KeySuper = false;
            bool kek = true;

            imgRenderer.beginFrame(backbuffer);

            ::ImGui::ShowTestWindow(&kek);

            ImGui::SetNextWindowPos({ 10.f, 10.f });
            if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
				if (ImGui::Button("Api"))
					ImGui::OpenPopup("selectApi");
				ImGui::SameLine();
				ImGui::Text(api == GraphicsApi::DX12 ? "DX12" : "Vulkan");
				if (ImGui::BeginPopup("selectApi"))
				{
					if (ImGui::Selectable("DX12"))
					{
						api = GraphicsApi::DX12;
						reInit = true;
						closeAnyway = true;
					}
					if (ImGui::Selectable("Vulkan"))
					{
						api = GraphicsApi::Vulkan;
						reInit = true;
						closeAnyway = true;
					}
					ImGui::EndPopup();
				}

				if (ImGui::Button("Device"))
					ImGui::OpenPopup("selectGpu");
				ImGui::SameLine();
				ImGui::Text(gpus[chosenGpu].name.c_str());
				if (ImGui::BeginPopup("selectGpu"))
				{
					for (auto&& it : gpus)
					{
						if (ImGui::Selectable(it.name.c_str()))
						{
							chosenGpu = it.id;
							reInit = true;
							closeAnyway = true;
							explicitID = true;
						}
					}
					ImGui::EndPopup();
				}

              ImGui::Text("FPS %.2f", 1000.f / time.getCurrentFps());
              ImGui::Text("Swapchain");
              ImGui::Text("format %s", formatToString(scdesc.desc.format));
              if (ImGui::Button(formatToString(FormatType::Unorm8x4)))
              {
                scdesc.desc.format = FormatType::Unorm8x4;
                toggleHDR = true;
              } ImGui::SameLine();
              if (ImGui::Button(formatToString(FormatType::Unorm10x3)))
              {
                scdesc.desc.format = FormatType::Unorm10x3;
                toggleHDR = true;
              } ImGui::SameLine();
              if (ImGui::Button(formatToString(FormatType::Float16x4)))
              {
                scdesc.desc.format = FormatType::Float16x4;
                toggleHDR = true;
              }
              ImGui::Text("Resolution: %dx%d", swapchain.impl()->desc().desc.width, swapchain.impl()->desc().desc.height);
              ImGui::Text("Displaycurve: %s", displayCurveToStr(swapchain.impl()->displayCurve()));
              ImGui::Text("%s", swapchain.impl()->HDRSupport() ? "HDR Available" : "HDR not supported");
              ImGui::Text("PresentMode: %s", presentModeToStr(scdesc.desc.mode));
              if (ImGui::Button(presentModeToStr(PresentMode::FifoRelaxed)))
              {
                scdesc.desc.mode = PresentMode::FifoRelaxed;
                toggleHDR = true;
              } ImGui::SameLine();
              if (ImGui::Button(presentModeToStr(PresentMode::Fifo)))
              {
                scdesc.desc.mode = PresentMode::Fifo;
                toggleHDR = true;
              } ImGui::SameLine();
              if (ImGui::Button(presentModeToStr(PresentMode::Immediate)))
              {
                scdesc.desc.mode = PresentMode::Immediate;
                toggleHDR = true;
              } ImGui::SameLine();
              if (ImGui::Button(presentModeToStr(PresentMode::Mailbox)))
              {
                scdesc.desc.mode = PresentMode::Mailbox;
                toggleHDR = true;
              }
              ImGui::Text("Colorspace: %s", colorspaceToStr(scdesc.desc.colorSpace));
              if (ImGui::Button(colorspaceToStr(Colorspace::BT709)))
              {
                scdesc.desc.colorSpace = Colorspace::BT709;
                toggleHDR = true;
              } ImGui::SameLine();
              if (ImGui::Button(colorspaceToStr(Colorspace::BT2020)))
              {
                scdesc.desc.colorSpace = Colorspace::BT2020;
                toggleHDR = true;
              }
              ImGui::SliderInt("Buffer count", &scdesc.desc.bufferCount, 2, 8);

              ImGui::Text("raymarch texture size %dx%d", ires.x(), ires.y());

              int oldRes = raymarch_Res;
              int oldraymarch_x = raymarch_x;
              int oldraymarch_y = raymarch_y;
              ImGui::SliderInt("scale X", &raymarch_x, 1, 30); ImGui::SameLine();
              ImGui::SliderInt("Y", &raymarch_y, 1, 30);	ImGui::SameLine();
              ImGui::SliderInt("max", &raymarch_Res, 1, 4096);

              ImGui::Checkbox("dynamic raymarch scale", &raymarchDynamicEnabled); ImGui::SameLine();
              raymarchfpsTarget = 1000.f / raymarchfpsTarget;
              ImGui::SliderFloat("target framerate", &raymarchfpsTarget, 10.f, 999.f);	ImGui::SameLine();
              raymarchfpsTarget = 1000.f / raymarchfpsTarget;
              ImGui::SliderFloat("threshold", &raymarchFpsThr, 0.001f, 1.f);

              if (ImGui::Button("reset to 30fps"))
              {
                raymarchfpsTarget = 16.666667f * 2.f;
              }
			  ImGui::SameLine();
			  if (ImGui::Button("reset to 60fps"))
			  {
				  raymarchfpsTarget = 16.666667f;
			  }
			  ImGui::SameLine();
			  if (ImGui::Button("reset to 120fps"))
			  {
				  raymarchfpsTarget = 16.666667f / 2.f;
			  }

              if (raymarchDynamicEnabled)
              {
                if (frame % 30 == 0)
                {
                  DynamicScale raymarchDynamicScale(raymarch_Res, raymarchfpsTarget, raymarchFpsThr);
                  raymarch_Res = raymarchDynamicScale.tick(time.getCurrentFps());
                }
              }

              if (raymarch_x > raymarch_y)
              {
                ires.data[0] = raymarch_Res;
                ires.data[1] = static_cast<int>(static_cast<float>(raymarch_y) / static_cast<float>(raymarch_x) * static_cast<float>(raymarch_Res));
              }
              else
              {
                ires.data[1] = raymarch_Res;
                ires.data[0] = static_cast<int>(static_cast<float>(raymarch_x) / static_cast<float>(raymarch_y) * static_cast<float>(raymarch_Res));
              }

              if (ImGui::Button("Reinit raymarch texture") || raymarch_Res != oldRes || oldraymarch_x != raymarch_x || oldraymarch_y != raymarch_y)
              {
                ires.data[0] = std::max(ires.data[0], 1);
                ires.data[1] = std::max(ires.data[1], 1);
                texture = dev.createTexture(ResourceDescriptor()
                  .setName("testTexture")
                  .setFormat(FormatType::Unorm16x4)
                  .setWidth(ires.data[0])
                  .setHeight(ires.data[1])
                  .setMiplevels(4)
                  .setDimension(FormatDimension::Texture2D)
                  .setUsage(ResourceUsage::RenderTargetRW));

                texRtv = dev.createTextureRTV(texture, ShaderViewDescriptor().setMostDetailedMip(0));
                texSrv = dev.createTextureSRV(texture, ShaderViewDescriptor().setMostDetailedMip(0).setMipLevels(1));
                texUav = dev.createTextureUAV(texture, ShaderViewDescriptor().setMostDetailedMip(0).setMipLevels(1));
              }
            }
            ImGui::End();

            imgRenderer.endFrame(dev, tasks, backbuffer);
          }

          {
            auto& node = tasks.createPass2("present");
            node.prepareForPresent(backbuffer);
          }

          dev.submit(swapchain, tasks);

          dev.present(swapchain);
        }
      }
      if (!reInit)
        break;
      else
      {
        reInit = false;
	  }
    }
  };
#if 0
  main(GraphicsApi::DX12, VendorID::Amd, true);
#else
  
   LBS lbs;
   lbs.addTask("test1", [&](size_t, size_t) {main(GraphicsApi::DX12, VendorID::Amd, true); });
   //lbs.addTask("test2", [&](size_t, size_t) {main(GraphicsApi::DX12, VendorID::Nvidia, false); });
   lbs.sleepTillKeywords({"test1"});
   
#endif
  log.update();
  return 1;
}
