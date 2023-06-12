#pragma once
#include "imgui/imgui.h"
#include "Utils.h"

#pragma warning (disable : 4244) //conversion from int to float

using ID = unsigned long long int;
static ID g_id = 0;

class Entity
{
public:
	enum class HealthState : int
	{
		Sick = 0,
		Infected,
		Recovering,
		Healthy,
		Count
	};

	explicit Entity(const ImVec2& boardDim, float drawSize)
	{
		m_id = g_id++;
		m_pos = ImVec2(Utils::Random(drawSize, boardDim.x - drawSize), Utils::Random(drawSize, boardDim.y - drawSize));
		m_velocity = Utils::Random(1, 3);
		m_angle = static_cast<float>(Utils::Random(0, 360));
		m_state = (HealthState)Utils::Random(0, int(HealthState::Count) - 1);
		m_age = Utils::Random(0, 60);
		UpdateStateTimer();
		m_preUpdateAge = m_age;
		m_isColliding = false;
		m_collidingWith = 0;
		m_prevState = m_state;
		m_nextState = m_state;
		m_immuneLevel =
			((m_age >= 0 && m_age < 15) || (m_age >= 70 && m_age <= 100)) ? static_cast<float>(Utils::Random(0, 3)) :
			(m_age >= 40 && m_age < 70) ? static_cast<float>(Utils::Random(4, 6)) :
			(m_age >= 30 && m_age < 60) ? static_cast<float>(Utils::Random(7, 10)) : 10.f;
	}
	explicit Entity(const ImVec2& boardDim, int age, float drawSize, HealthState state, float immuneLevel) : 
		m_age(age), m_state(state), m_immuneLevel(immuneLevel), m_preUpdateAge(age), m_id(g_id++)
	{
		m_pos = ImVec2(Utils::Random(drawSize, boardDim.x - drawSize), Utils::Random(drawSize, boardDim.y - drawSize));
		m_velocity = Utils::Random(1, 3);
		m_angle = static_cast<float>(Utils::Random(0, 360));
		m_immuneLevel = std::clamp(m_immuneLevel, 0.0f, 10.0f);
		m_isColliding = false;
		m_collidingWith = 0;
		m_prevState = m_state;
		m_nextState = m_state;
		UpdateStateTimer();
	}
	void Update()
	{
		++m_age;
		if (int ageGroup = HasChangedAgeGroup(); ageGroup != -1)
		{
			if (ageGroup == 0) //niska
				m_immuneLevel = m_immuneLevel > 3.f ? 3.f : m_immuneLevel;
			else if (ageGroup == 1) //œrednia
				m_immuneLevel = m_immuneLevel > 6.f ? 6.f : m_immuneLevel;
			else if (ageGroup == 2) //wysoka
				m_immuneLevel = m_immuneLevel > 10.f ? 10.f : m_immuneLevel;
		}

		if (m_state == HealthState::Infected)
			m_nextState = HealthState::Sick;
		else if (m_state == HealthState::Sick)
			m_nextState = HealthState::Recovering;
		else if (m_state == HealthState::Recovering)
			m_nextState = HealthState::Healthy;
		else
			m_nextState = HealthState::Healthy;

		if (m_prevState != m_state)
		{
			m_prevState = m_state;
			UpdateStateTimer();
		}

		if (m_stateTime-- <= 0)
		{
			m_state = m_nextState;
		}

		m_preUpdateAge = m_age;
	}

	ID				GetId() const								{ return m_id; }
	ID				GetColliderId() const						{ return m_collidingWith; }
	void			SetColliderId(ID id)						{ m_collidingWith = id; }
	bool			IsColliding() const							{ return m_isColliding; }
	void			SetColliding(bool isColliding)				{ m_isColliding = isColliding; }
	void			ResetStateTime()							{ m_stateTime = 7; }

	int				m_velocity;				// 1 - 3
	float			m_angle;				// 0.f - 360.f
	int				m_age;					// 0 - 100
	float			m_immuneLevel;			// 0 - 10
	ImVec2			m_pos;					// x, y
	HealthState		m_state;				// Sick, Infected, Recovering, Healthy

private:
	int				HasChangedAgeGroup()
	{
		auto Between = [](int value, int min, int max) { return (value >= min && value <= max); };

		if (Between(m_preUpdateAge, 0, 14) && m_age > 14)
			return 2; //wysoka

		if (Between(m_preUpdateAge, 40, 69) && m_age > 69)
			return 0; //niska

		if (Between(m_preUpdateAge, 15, 39) && m_age > 39)
			return 1; //œrednia

		return -1;
	}
	void			UpdateStateTimer()
	{
		if (m_state == HealthState::Infected)
			m_stateTime = 2;
		else if (m_state == HealthState::Sick)
			m_stateTime = 7;
		else if (m_state == HealthState::Recovering)
			m_stateTime = 5;
	}

	int				m_preUpdateAge;
	ID				m_id;
	ID				m_collidingWith;
	bool			m_isColliding;

	int				m_stateTime;
	HealthState		m_prevState;
	HealthState		m_nextState;
};