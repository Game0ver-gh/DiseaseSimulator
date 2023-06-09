#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <string>
#include <functional>
#include <stdexcept>
#include "imgui/imgui.h"

class Window
{
public:
	static Window* GetInstance(const std::wstring& name = L"Window", const ImVec2& pos = { 100.f, 100.f }, const ImVec2& size = { 800.f, 600.f })
	{
		static Window instance(name, pos, size);
		return &instance;
	}
	~Window();;
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void Run(std::function<bool()> loop_callback);

private:
	struct FrameContext
	{
		ID3D12CommandAllocator* CommandAllocator;
		UINT64                  FenceValue;
	};

	Window(const std::wstring& name, const ImVec2& pos, const ImVec2& size) :
		m_name(name), m_pos(pos), m_size(size)
	{
		if (CreateMainWindow())
			InitImGui();
		else 
			throw std::runtime_error("Failed to create window");
	}
	bool CreateMainWindow();
	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
	void DestroyMainWindow();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void WaitForLastSubmittedFrame();
	FrameContext* WaitForNextFrameResources();

	void InitImGui();
	void CleanupImGui();
	
	std::wstring m_name;
	ImVec2 m_pos;
	ImVec2 m_size;
	ImVec4	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	//DirectX Data
	HWND							m_hWnd;
	FrameContext					g_frameContext[3] = {};
	UINT							g_frameIndex = 0;
	ID3D12Device*					g_pd3dDevice = nullptr;
	ID3D12DescriptorHeap*			g_pd3dRtvDescHeap = nullptr;
	ID3D12DescriptorHeap*			g_pd3dSrvDescHeap = nullptr;
	ID3D12CommandQueue*				g_pd3dCommandQueue = nullptr;
	ID3D12GraphicsCommandList*		g_pd3dCommandList = nullptr;
	ID3D12Fence*					g_fence = nullptr;
	HANDLE							g_fenceEvent = nullptr;
	UINT64							g_fenceLastSignaledValue = 0;
	IDXGISwapChain3*				g_pSwapChain = nullptr;
	HANDLE							g_hSwapChainWaitableObject = nullptr;
	ID3D12Resource*					g_mainRenderTargetResource[3] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE		g_mainRenderTargetDescriptor[3] = {};
};

