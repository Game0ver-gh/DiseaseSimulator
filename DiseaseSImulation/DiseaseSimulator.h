#pragma once
#include <random>
#include <vector>
#include <chrono>
#include <optional>
#include "Entity.h"

using namespace std::chrono_literals;
using namespace std::chrono;

class DiseaseSimulator
{
public:
	struct Params
	{
		uint32_t				numEntities;
		uint32_t				numRounds;
		milliseconds			simTime;
	};

	explicit					DiseaseSimulator(const Params& params);

	bool						Draw();

private:
	using Time =				std::chrono::milliseconds;
	using Colliders =			std::pair<Entity*, Entity*>;

	void						DrawGrid();
	void						DrawPlots();
	void						DrawToolWindow();
	void 						DrawEntity(const Entity& ent);

	void						RunSimulation();
	void						ResetSimulation();
	void						UpdateScreen();
	void						UpdateTick();

	bool						CreateEntities();
	const ImColor& 				GetEntityColor(Entity::HealthState state) const;
	int							GetMaxImmuneByAgeGroup(int ageGroup) const;
	int							GetMinImmuneByAgeGroup(int ageGroup) const;
	int							GetAgeGroup(int age) const;

	void						HandleEntityMove(Entity& ent);
	std::optional<Colliders>	HandleEntityCollision(Entity& ent);
	void						HandleEntityBorn(const Colliders& colliders, uint32_t* newBornCount);
	void						HandleEntityInfection(const Colliders& colliders);
	void						HandleEntityDeath(const std::vector<int>& deadEnts);

	Time						m_roundTime;
	Time						m_spawnTime;
	ImDrawList* 				m_drawList;
	uint32_t					m_curSimStage;
	uint32_t					m_curRound;
	uint32_t					m_numEntities;
	int							m_bornChance;
	int							m_numRounds;
	ImVec2						m_displaySize;
	ImVec2						m_simBoardSize;
	std::vector<Entity>			m_entities;
	int							m_initialEntityAge;

	const float					m_entityDrawRadius;
	const uint32_t				m_toolWindowWidth;
	const ImColor				m_entityStateColors[uint32_t(Entity::HealthState::Count)] = 
	{
		ImColor(255, 0, 0),		//Sick
		ImColor(255, 255, 0),	//Infected
		ImColor(255, 165, 0),	//Recovering
		ImColor(0, 255, 0)		//Healthly
	};

	std::vector<int>			m_data_y;
	std::vector<int>			m_data_healthly_x;
	std::vector<int>			m_data_infected_x;
	std::vector<int>			m_data_recovered_x;
	std::vector<int>			m_data_sick_x;
	std::vector<int>			m_data_dead_x;
	std::vector<int>			m_data_newborn_x;
};