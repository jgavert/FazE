#include "windowMain.hpp"
#include <higanbana/core/system/LBS.hpp>
#include <higanbana/core/filesystem/filesystem.hpp>
#include <higanbana/core/Platform/Window.hpp>
#include <higanbana/core/system/logger.hpp>
#include <higanbana/core/system/time.hpp>
#include <higanbana/core/global_debug.hpp>
#include <higanbana/core/entity/bitfield.hpp>
#include <higanbana/core/system/misc.hpp>
#include <higanbana/graphics/GraphicsCore.hpp>
#include <random>
#include <tuple>

#include "entity_test.hpp"
#include "rendering.hpp"

using namespace higanbana;
using namespace higanbana::math;

vector<std::string> splitByDelimiter(std::string data, const char* delimiter)
{
  vector<std::string> splits;
  
  size_t pos = 0;
  std::string token;
  while ((pos = data.find(delimiter)) != std::string::npos) {
    token = data.substr(0, pos);
    splits.emplace_back(token);
    data.erase(0, pos + strlen(delimiter));
  }
  return splits;
}

vector<int> convertToInts(vector<std::string> data)
{
  vector<int> ints;
  for (auto&& s : data)
  {
    try
    {
      auto val = std::stoi(s.c_str());
      ints.push_back(val);
    } catch (...)
    {
    }
  }
  return ints;
}

void mainWindow(ProgramParams& params)
{
  GraphicsApi cmdlineApiId = GraphicsApi::Vulkan;
  VendorID cmdlineVendorId = VendorID::Amd;
  auto cmdLength = strlen(params.m_lpCmdLine);
  F_ILOG("", "cmdline: \"%s\"", params.m_lpCmdLine);
  std::string cmdline = params.m_lpCmdLine;
  std::replace(cmdline.begin(), cmdline.end(), '"', ' ');
  std::replace(cmdline.begin(), cmdline.end(), '\\', ' ');
  F_ILOG("", "cmdline: \"%s\"", cmdline.c_str());
  auto splits = splitByDelimiter(cmdline, " ");
  auto ints = convertToInts(splits);
  if (ints.size() > 0)
  {
    cmdlineApiId = static_cast<GraphicsApi>(ints[0]); 
  }
  if (ints.size() > 1)
  {
    cmdlineVendorId = static_cast<VendorID>(ints[1]); 
  }
  Logger log;
  // test_entity();
  // test_bitfield();
  auto main = [&](GraphicsApi api, VendorID preferredVendor, bool updateLog)
  {
    F_LOG("Trying to start with %s api and %s vendor\n", toString(api), toString(preferredVendor));
    bool reInit = false;
    int64_t frame = 1;
    FileSystem fs("/../../data");
    bool quit = false;

    log.update();

    bool explicitID = false;
    //GpuInfo gpuinfo{};

    while (true)
    {
      vector<GpuInfo> allGpus;
      GraphicsSubsystem graphics("higanbana", true);
      auto gpus = graphics.availableGpus();
#if 1
      auto gpuInfo = graphics.getVendorDevice(api, preferredVendor);
      auto api2 = GraphicsApi::Vulkan;
      if (api == GraphicsApi::Vulkan)
      {
        api2 = GraphicsApi::DX12;
      }
      auto gpuInfo2 = graphics.getVendorDevice(api2, preferredVendor);
      allGpus.emplace_back(gpuInfo);
      //allGpus.emplace_back(gpuInfo2);
#else
      if (!explicitID)
      {
        //gpuinfo = graphics.getVendorDevice(api);
        for (auto&& it : gpus)
        {
          if (it.api == api && it.vendor == preferredVendor)
          {
            allGpus.push_back(it);
            break;
          }
          F_LOG("\t%s: %d. %s (memory: %zdMB, api: %s)\n", toString(it.api), it.id, it.name.c_str(), it.memory/1024/1024, it.apiVersionStr.c_str());
        }
      }
      if (allGpus.empty())
      {
        for (auto&& it : gpus)
        {
          if (it.api == api)
          {
            allGpus.push_back(it);
            break;
          }
          F_LOG("\t%s: %d. %s (memory: %zdMB, api: %s)\n", toString(it.api), it.id, it.name.c_str(), it.memory/1024/1024, it.apiVersionStr.c_str());
        }
      }
#endif
      if (updateLog) log.update();
      if (gpus.empty())
        return;

	    std::string windowTitle = "";
      for (auto& gpu : allGpus)
      {
        windowTitle += std::string(toString(gpu.api)) + ": " + gpu.name + " ";
      }
      Window window(params, windowTitle, 1280, 720, 300, 200);
      window.open();

      auto dev = graphics.createGroup(fs, allGpus);
      app::Renderer rend(graphics, dev);
      {
        auto toggleHDR = false;
        rend.initWindow(window, allGpus[0]);

        for (auto& gpu : allGpus)
        {
          F_LOG("Created device \"%s\"\n", gpu.name.c_str());
        }

        bool closeAnyway = false;
        bool captureMouse = false;
        bool controllerConnected = false;

        while (!window.simpleReadMessages(frame++))
        {
          // update inputs and our position
          //directInputs.pollDevices(gamepad::Fic::PollOptions::AllowDeviceEnumeration);
          // update fs
          log.update();
          fs.updateWatchedFiles();
          if (window.hasResized() || toggleHDR)
          {
            //dev.adjustSwapchain(swapchain, scdesc);
            rend.windowResized();
            window.resizeHandled();
            toggleHDR = false;
          }
          auto& inputs = window.inputs();

          if (inputs.isPressedThisFrame(VK_F1, 1))
          {
            window.captureMouse(true);
            captureMouse = true;
          }
          if (inputs.isPressedThisFrame(VK_F2, 1))
          {
            window.captureMouse(false);
            captureMouse = false;
          }

          if (inputs.isPressedThisFrame(VK_MENU, 2) && inputs.isPressedThisFrame('1', 1))
          {
            window.toggleBorderlessFullscreen();
          }

          if (inputs.isPressedThisFrame(VK_MENU, 2) && inputs.isPressedThisFrame('2', 1))
          {
            reInit = true;
            if (api == GraphicsApi::DX12)
              api = GraphicsApi::Vulkan;
            else
              api = GraphicsApi::DX12;
            break;
          }

          if (inputs.isPressedThisFrame(VK_MENU, 2) && inputs.isPressedThisFrame('3', 1))
          {
            reInit = true;
            if (preferredVendor == VendorID::Amd)
              preferredVendor = VendorID::Nvidia;
            else
              preferredVendor = VendorID::Amd;
            break;
          }

          if (frame > 10 && (closeAnyway || inputs.isPressedThisFrame(VK_ESCAPE, 1)))
          {
            break;
          }
          if (updateLog) log.update();

          rend.render();
        }
        dev.waitGpuIdle();
      }
      if (!reInit)
        break;
      else
      {
        reInit = false;
      }
    }
    quit = true;
  };
#if 0
  main(GraphicsApi::DX12, VendorID::Amd, true);
#else
  main(cmdlineApiId, cmdlineVendorId, true);
  //lbs.addTask("test1", [&](size_t) {main(GraphicsApi::Vulkan, VendorID::Nvidia, true); });
  //lbs.sleepTillKeywords({ "test1" });

#endif
  /*
  RangeMath::difference({ 0, 5, 0, 4 }, { 2, 1, 1, 2 }, [](SubresourceRange r) {printRange(r);});
  RangeMath::difference({0, 5, 1, 2}, {2, 1, 0, 5}, [](SubresourceRange r) {printRange(r);});
  RangeMath::difference({1, 2, 0, 4}, {0, 5, 1, 2}, [](SubresourceRange r) {printRange(r);});
  RangeMath::difference({2, 1, 1, 2}, {0, 5, 0, 4}, [](SubresourceRange r) {printRange(r);});
  RangeMath::difference({ 0, 2, 0, 2 }, { 1, 1, 1, 1 }, [](SubresourceRange r) {printRange(r);});
  RangeMath::difference({ 0, 1, 0, 1 }, { 1, 1, 1, 1 }, [](SubresourceRange r) {printRange(r);});
  */

  log.update();
}
