#include <memory>
#include "Window.h"
#include "DiseaseSimulator.h"

//Hide console window
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int argc, char* argv[])
{
	auto window = std::make_unique<Window>(L"Disease simulator", ImVec2(1300, 700));
	auto simulator = std::make_unique<DiseaseSimulator>(DiseaseSimulator::Params
		{
			.numEntities	= 128,
			.numRounds		= 230,
			.simTime		= 150ms
		});

	window->Run([&simulator]() { return simulator->Draw(); });

	return 0;
}