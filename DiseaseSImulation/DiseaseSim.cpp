#include <memory>
#include "Window.h"
#include "DiseaseSimulator.h"

int main(int argc, char* argv[])
{
	auto window = std::make_unique<Window>(L"Disease simulator", ImVec2(1600, 900));
	auto simulator = std::make_unique<DiseaseSimulator>(DiseaseSimulator::Params
		{
			128,	// Number of entities
			230,	// Number of rounds
			150ms	// Round time
		});

	window->Run([&simulator]() { return simulator->Draw(); });

	return 0;
}