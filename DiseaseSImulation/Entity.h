#pragma once
#include "imgui/imgui.h"
#include "Utils.h"

#pragma warning (disable : 4244) //conversion from int to float

using ID = long long unsigned int;
static ID g_id = 0;

class Entity
{
public:
	enum class		HealthState : int
	{
		Sick = 0,
		Infected,
		Recovering,
		Healthy,
		Count
	};

	explicit		Entity(const ImVec2& boardDim, float drawSize);
	explicit		Entity(const ImVec2& boardDim, int age, float drawSize, HealthState state, float immuneLevel);
	void			Update();

	ID				GetId() const;
	ID				GetColliderId() const;
	void			SetColliderId(ID id);
	bool			IsColliding() const;
	void			SetColliding(bool isColliding);
	void			ResetStateTime();

	int				m_velocity;				// 1 - 3
	float			m_angle;				// 0.f - 360.f
	int				m_age;					// 0 - 100
	float			m_immuneLevel;			// 0 - 10
	ImVec2			m_pos;					// x, y
	HealthState		m_state;				// Sick, Infected, Recovering, Healthy

private:
	int				HasChangedAgeGroup();
	void			UpdateStateTimer();

	int				m_preUpdateAge;
	ID				m_id;
	ID				m_collidingWith;
	bool			m_isColliding;

	int				m_stateTime;
	HealthState		m_prevState;
	HealthState		m_nextState;
};