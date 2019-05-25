#pragma once
#include "core/platform/definitions.hpp"
#if defined(FAZE_PLATFORM_WINDOWS)
#define FAZE_DX12_DXIL

#include <DXGI.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <dxc/dxcapi.h>
#pragma warning( push )
#pragma warning( disable : 4100)
#include <dxc/Support/microcom.h>
#pragma warning( pop )
#include <wrl.h>
#include <Objbase.h>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using D3D12Swapchain = IDXGISwapChain4;

#endif