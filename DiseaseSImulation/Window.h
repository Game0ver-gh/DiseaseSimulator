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
	explicit Window(const std::wstring& name, const ImVec2& size = ImVec2(1600, 900)) : m_name(name), m_size(size)
	{
		try
		{
			if (CreateMainWindow()) 
				InitImGui();
		}
		catch (const std::runtime_error& e)
		{
			DestroyMainWindow();
			throw e;
		}
	}
	~Window();
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Main window loop for draw calls. Return true in loop_callback to exit.
	void Run(std::function<bool()> loop_callback);

private:
	bool CreateMainWindow();
	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
	void DestroyMainWindow();

	void InitImGui();
	void CleanupImGui();
	
	std::wstring					m_name;
	ImVec2							m_size;
	ImVec4							m_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

