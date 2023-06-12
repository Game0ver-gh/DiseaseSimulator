#pragma once
#include <random>
#include <chrono>

class Utils
{
public:
	static int Random(int min, int max)
	{
		static std::random_device rd;
		static std::mt19937 generator(rd());
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(generator);
	}
	static float Distance(const ImVec2& a, const ImVec2& b)
	{
		auto x = a.x - b.x;
		auto y = a.y - b.y;
		return sqrtf(x * x + y * y);
	}
};

class Timer
{
public:
	Timer()
	{
		m_start = std::chrono::high_resolution_clock::now();
	}

	bool HasExpired(std::chrono::milliseconds interval)
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start);
		if (elapsed >= interval)
		{
			m_start = now;
			return true;
		}
		return false;
	}

private:
	std::chrono::time_point<std::chrono::steady_clock>		m_start;
};