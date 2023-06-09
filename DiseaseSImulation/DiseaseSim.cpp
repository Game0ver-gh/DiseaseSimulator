#include "Window.h"

#include <chrono>

int main(int argc, char* argv[])
{
	auto* window = Window::GetInstance(L"Test1");
	auto start = std::chrono::high_resolution_clock::now();

	window->Run([&]() -> bool
		{
			auto end = std::chrono::high_resolution_clock::now();
			if (end - start > std::chrono::seconds(3))
			{
				return true;
			}

			return false;
		});

	return 0;
}