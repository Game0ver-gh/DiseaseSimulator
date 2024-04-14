#include "Entity.h"

Entity::Entity(const ImVec2& boardDim, float drawSize)
{
	m_id = g_id++;
	m_pos = ImVec2(Utils::Random(drawSize, boardDim.x - drawSize), Utils::Random(drawSize, boardDim.y - drawSize));
	m_velocity = Utils::Random(1, 3);
	m_angle = static_cast<float>(Utils::Random(0, 360));
	m_state = static_cast<HealthState>(Utils::Random(0, int(HealthState::Count) - 1));
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

Entity::Entity(const ImVec2& boardDim, int age, float drawSize, HealthState state, float immuneLevel) :
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

void Entity::Update()
{
	++m_age;
	if (int ageGroup = HasChangedAgeGroup(); ageGroup != -1)
	{
		if (ageGroup == 0) //low immune
			m_immuneLevel = m_immuneLevel > 3.f ? 3.f : m_immuneLevel;
		else if (ageGroup == 1) //medium immune
			m_immuneLevel = m_immuneLevel > 6.f ? 6.f : m_immuneLevel;
		else if (ageGroup == 2) //high immune
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

ID Entity::GetId() const { return m_id; }

ID Entity::GetColliderId() const { return m_collidingWith; }

void Entity::SetColliderId(ID id) { m_collidingWith = id; }

bool Entity::IsColliding() const { return m_isColliding; }

void Entity::SetColliding(bool isColliding) { m_isColliding = isColliding; }

void Entity::ResetStateTime() { m_stateTime = 7; }

int Entity::HasChangedAgeGroup()
{
	auto Between = [](int value, int min, int max) { return (value >= min && value <= max); };

	if (Between(m_preUpdateAge, 0, 14) && m_age > 14)
		return 2; //High immune

	if (Between(m_preUpdateAge, 40, 69) && m_age > 69)
		return 0; //Low immune

	if (Between(m_preUpdateAge, 15, 39) && m_age > 39)
		return 1; //Medium immune

	return -1;
}

void Entity::UpdateStateTimer()
{
	if (m_state == HealthState::Infected)
		m_stateTime = 2;
	else if (m_state == HealthState::Sick)
		m_stateTime = 7;
	else if (m_state == HealthState::Recovering)
		m_stateTime = 5;
}
