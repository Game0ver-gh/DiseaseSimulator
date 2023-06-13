#include "DiseaseSimulator.h"
#include "imgui/implot.h"
#include <cmath>
#include <thread>
#include <unordered_set>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define IMGUI_TOOLWINDOW_FLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)
#define IMGUI_PLOTWINDOW_FLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)
#define MAX_ENTITIES 2000
#pragma warning (disable : 4267) //conversion from size_t to int

static bool run = false;
static bool done = false;
static bool randomInitialAge = true;

unsigned long long int tick = 0;

DiseaseSimulator::DiseaseSimulator(const Params& params) : m_toolWindowWidth(400), m_entityDrawRadius(6.f)
{
    m_drawList = nullptr;
    m_numEntities = params.numEntities;
    m_numRounds = params.numRounds;
    m_roundTime = params.simTime;
    m_spawnTime = m_roundTime / m_numEntities;
    m_curSimStage = 0;
    m_curRound = 0;
    m_initialEntityAge = 32;
    m_bornChance = 50;

    auto& io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Tahoma.ttf", 22.0f);
    if (font) io.FontDefault = font;
}

void DiseaseSimulator::RunSimulation()
{
    if (run)
    {
        static Timer sim_interval;
        if (m_curRound == m_numRounds)
        {
            done = true;
            tick = 0;
            return;
        }

        if (m_curSimStage == 1 && m_entities.empty())
        {
            done = true;
            tick = 0;
            return;
        }

        if (m_curSimStage == 0)
        {
            if (CreateEntities())
                m_curSimStage++;
        }
        else if (m_curSimStage == 1)
        {
            bool newRound = false;
            if (sim_interval.HasExpired(m_roundTime))
            {
                m_curRound++;
                newRound = true;
            }

            unsigned int 
                infected = 0, healthly = 0, 
                sick = 0, recovering = 0,
                death = 0, newBorn = 0;

            std::vector<int> deadEntities;
            for (auto& entity : m_entities)
            {
                if (newRound)
                {
                    entity.Update();

                    float maxImmune = ((entity.m_age >= 0 && entity.m_age < 15) || (entity.m_age >= 70 && entity.m_age <= 100)) ? 3.0f :
                        (entity.m_age >= 40 && entity.m_age < 70) ? 6.0f :
                        (entity.m_age >= 30 && entity.m_age < 60) ? 10.0f : 10.0f;

                    if (entity.m_state == Entity::HealthState::Infected)
                    {
                        entity.m_immuneLevel -= 0.1f;
                        
                    }
                    else if (entity.m_state == Entity::HealthState::Sick)
                    {
                        entity.m_immuneLevel -= 0.5f;
                        
                    }
                    else if (entity.m_state == Entity::HealthState::Recovering)
                    {
                        entity.m_immuneLevel += 0.1f;
                        entity.m_immuneLevel = std::clamp(entity.m_immuneLevel, 0.f, maxImmune);
                        
                    }
                    else if (entity.m_state == Entity::HealthState::Healthy)
                    {
                        entity.m_immuneLevel += 0.05f;
                        entity.m_immuneLevel = std::clamp(entity.m_immuneLevel, 0.f, maxImmune);
                        
                    }

                }

                HandleEntityMovement(entity);
                auto colliders = HandleEntityCollision(entity); // O(n^2) :sadge:
                if (colliders.has_value())
                {
                    //colliders
                    HandleEntityBorn(colliders.value(), &newBorn);
                    HandleEntityInfection(colliders.value());
                }

                //death
                if (entity.m_age >= 100 || entity.m_immuneLevel <= 0.f)
                {
                    deadEntities.push_back(entity.GetId());
                    death++;
                }

                //collecting data
                if (entity.m_state == Entity::HealthState::Infected)
                {
                    infected++;
                }
                else if (entity.m_state == Entity::HealthState::Sick)
                {
                    sick++;
                }
                else if (entity.m_state == Entity::HealthState::Recovering)
                {
                    recovering++;
                }
                else if (entity.m_state == Entity::HealthState::Healthy)
                {
                    healthly++;
                }
            }

            m_data_infected_x.push_back(infected);
            m_data_sick_x.push_back(sick);
            m_data_recovered_x.push_back(recovering);
            m_data_healthly_x.push_back(healthly);
            m_data_dead_x.push_back(death);
            m_data_y.push_back(tick);
            m_data_newborn_x.push_back(newBorn);

            HandleEntityDeath(deadEntities);
        }
    }

    for (auto& entity : m_entities)
        DrawEntity(entity);
}

void DiseaseSimulator::ResetSimulation()
{
    m_entities.clear();
    m_curSimStage = 0;
    m_curRound = 0;
    done = false;
    run = false;
}

void DiseaseSimulator::UpdateScreen()
{
    m_drawList = ImGui::GetBackgroundDrawList();
    m_displaySize = ImGui::GetIO().DisplaySize;
    m_simBoardSize = ImVec2(m_displaySize.x - m_toolWindowWidth, m_displaySize.y);
}

void DiseaseSimulator::UpdateTick()
{
    if (tick++ >= std::numeric_limits<unsigned long long int>::max())
        tick = 0;
}

bool DiseaseSimulator::CreateEntities()
{
    static Timer interval;

    if (interval.HasExpired(m_spawnTime))
    {
        m_entities.emplace_back(
            Entity(ImVec2(m_simBoardSize.x + 1.f, m_simBoardSize.y - 1.f), m_entityDrawRadius));
    }

    return m_entities.size() == m_numEntities;
}

const ImColor& DiseaseSimulator::GetEntityColor(Entity::HealthState state) const
{
    assert(int(state) > -1 && int(state) < int(Entity::HealthState::Count));
    return m_entityStateColors[uint32_t(state)];
}

void DiseaseSimulator::HandleEntityMovement(Entity& ent)
{
    ImVec2 displacement1 =
    {
        ent.m_velocity * std::cosf(ent.m_angle * M_PI / 180.0f),
        ent.m_velocity * std::sinf(ent.m_angle * M_PI / 180.0f),
    };

    ent.m_pos.x += displacement1.x;
    ent.m_pos.y += displacement1.y;
}

std::optional<DiseaseSimulator::Colliders> DiseaseSimulator::HandleEntityCollision(Entity& ent)
{
    bool isCollidingWithOther = false;
    DiseaseSimulator::Colliders colliders;

    if (ent.m_pos.x - m_entityDrawRadius < 0)
    {
        ent.m_pos.x = m_entityDrawRadius;
        ent.m_angle = 180.0f - ent.m_angle;
    }
    else if (ent.m_pos.x + m_entityDrawRadius > m_simBoardSize.x)
    {
        ent.m_pos.x = m_simBoardSize.x - m_entityDrawRadius;
        ent.m_angle = 180.0f - ent.m_angle;
    }

    if (ent.m_pos.y - m_entityDrawRadius < 0)
    {
        ent.m_pos.y = m_entityDrawRadius;
        ent.m_angle = -ent.m_angle;
    }
    else if (ent.m_pos.y + m_entityDrawRadius > m_simBoardSize.y)
    {
        ent.m_pos.y = m_simBoardSize.y - m_entityDrawRadius;
        ent.m_angle = -ent.m_angle;
    }

    for (auto& other : m_entities)
    {
        if (&ent == &other) // Skip self
            continue;

        if (Utils::Distance(ent.m_pos, other.m_pos) > m_entityDrawRadius * 2.0f)
        {
            ent.SetColliding(false);
			other.SetColliding(false);
			continue;
        }

        if ((ent.IsColliding() && other.GetColliderId() == ent.GetId()))
            continue;

        isCollidingWithOther = true;
        colliders = { &ent, &other };

        // Reflect
        float collisionAngle = std::atan2(other.m_pos.y - ent.m_pos.y, other.m_pos.x - ent.m_pos.x) * 180.0f / M_PI;
        ent.m_angle = 2.f * collisionAngle - ent.m_angle;

        ent.SetColliding(true);
        other.SetColliding(true);
        ent.SetColliderId(other.GetId());
        other.SetColliderId(ent.GetId());
    }

    return isCollidingWithOther ? std::make_optional(colliders) : std::nullopt;
}

void DiseaseSimulator::HandleEntityBorn(const Colliders& colliders, uint32_t* newBornCount)
{
    if (m_entities.size() >= MAX_ENTITIES) // Max entities
        return;

    bool ifHerAgeAintOnTheClockShesReadyForTheCock =
        colliders.first->m_age >= 20 && colliders.first->m_age <= 40 &&
        colliders.second->m_age >= 20 && colliders.second->m_age <= 40;

    if (ifHerAgeAintOnTheClockShesReadyForTheCock && Utils::Random(0, 100) < m_bornChance)
    {
        bool twins = Utils::Random(0, 100) == 1;
        int children = twins ? 2 : 1;
        *newBornCount = children;
        for (size_t i = 0; i < children; i++)
        {
            m_entities.emplace_back(
                Entity(
                    ImVec2(m_simBoardSize.x + 1.f, m_simBoardSize.y - 1.f),
                    0,
                    m_entityDrawRadius,
                    Entity::HealthState::Healthy,
                    3));
        }
    }
    else *newBornCount = 0;
}

void DiseaseSimulator::HandleEntityInfection(const Colliders& colliders)
{
    using State = Entity::HealthState;
    auto& first = *colliders.first;
    auto& second = *colliders.second;

    auto GetMaxImmunyByAgeGroup = [](int age) -> float
    {
        return ((age >= 0 && age < 15) || (age >= 70 && age <= 100)) ? 3.0f :
            (age >= 40 && age < 70) ? 6.0f :
            (age >= 30 && age < 60) ? 10.0f : 10.0f;
    };
    auto GetMinImmunyByAgeGroup = [](int age) -> float
    {
        return ((age >= 0 && age < 15) || (age >= 70 && age <= 100)) ? 0.0f :
            (age >= 40 && age < 70) ? 3.0f :
            (age >= 30 && age < 60) ? 6.0f : 6.0f;
    };
    auto GetAgeGroup = [](int age) -> int
    {
        return ((age >= 0 && age < 15) || (age >= 70 && age <= 100)) ? 0 :
            (age >= 40 && age < 70) ? 1 :
            (age >= 30 && age < 60) ? 2 : 2;
    };

    if (first.m_state == State::Healthy && second.m_state == State::Infected)
    {
        if (first.m_immuneLevel > 0.0f && first.m_immuneLevel <= 3.0f)
            first.m_state = State::Infected;
    }
    else if (first.m_state == State::Healthy && second.m_state == State::Sick)
    {
        if (first.m_immuneLevel > 0.0f && first.m_immuneLevel <= 6.0f)
            first.m_state = State::Infected;
        else if (first.m_immuneLevel > 6.0f && first.m_immuneLevel <= 10.0f)
            first.m_immuneLevel -= 3.0f;
	}
    else if (first.m_state == State::Healthy && second.m_state == State::Recovering)
    {
        second.m_immuneLevel += 1.0f;
    }
    else if (first.m_state == State::Healthy && second.m_state == State::Healthy)
    {
        if (first.m_immuneLevel < second.m_immuneLevel)
        {
            int ageGroupFirst = GetAgeGroup(first.m_age);
            int ageGroupSecond = GetAgeGroup(second.m_age);

            if (ageGroupFirst == ageGroupSecond)
                first.m_immuneLevel = std::max(first.m_immuneLevel, second.m_immuneLevel);
            else
                first.m_immuneLevel = GetMaxImmunyByAgeGroup(first.m_age);
        }
        else
        {
            int ageGroupFirst = GetAgeGroup(first.m_age);
            int ageGroupSecond = GetAgeGroup(second.m_age);

            if (ageGroupFirst == ageGroupSecond)
                second.m_immuneLevel = std::max(first.m_immuneLevel, second.m_immuneLevel);
            else
                second.m_immuneLevel = GetMaxImmunyByAgeGroup(second.m_age);
        }
    }
    else if (first.m_state == State::Sick && second.m_state == State::Infected)
    {
        if (second.m_immuneLevel > 0.0f && second.m_immuneLevel <= 6.0f)
            second.m_state = State::Sick;

        first.ResetStateTime();
    }
    else if (first.m_state == State::Sick && second.m_state == State::Recovering)
    {
        if (second.m_immuneLevel > 0.0f && second.m_immuneLevel <= 6.0f)
            second.m_state = State::Infected;
    }
    else if (first.m_state == State::Sick && second.m_state == State::Sick)
    {
        if (first.m_immuneLevel < second.m_immuneLevel)
        {
            int ageGroupFirst = GetAgeGroup(first.m_age);
            int ageGroupSecond = GetAgeGroup(second.m_age);

            if (ageGroupFirst == ageGroupSecond)
                first.m_immuneLevel = std::min(first.m_immuneLevel, second.m_immuneLevel);
            else
                first.m_immuneLevel = GetMinImmunyByAgeGroup(first.m_age);
        }
        else
        {
            int ageGroupFirst = GetAgeGroup(first.m_age);
            int ageGroupSecond = GetAgeGroup(second.m_age);

            if (ageGroupFirst == ageGroupSecond)
                second.m_immuneLevel = std::min(first.m_immuneLevel, second.m_immuneLevel);
            else
                second.m_immuneLevel = GetMinImmunyByAgeGroup(second.m_age);
        }

        first.ResetStateTime();
        second.ResetStateTime();
    }
    else if (first.m_state == State::Infected && second.m_state == State::Recovering)
    {
        second.m_immuneLevel -= 1.0f;
    }
    else if (first.m_state == State::Recovering && second.m_state == State::Recovering)
    {
        //nothing
    }
}

void DiseaseSimulator::HandleEntityDeath(const std::vector<int>& deadEnts)
{
    for (auto& id : deadEnts)
    {
        auto it = std::find_if(m_entities.begin(), m_entities.end(), [id](const Entity& ent) { return ent.GetId() == id; });
        if (it != m_entities.end())
            m_entities.erase(it);
    }
}

bool DiseaseSimulator::Draw()
{
    UpdateScreen();

    DrawToolWindow();

    if (run && done)
        DrawPlots();
    
    DrawGrid();
    
    RunSimulation();

    UpdateTick();

    return false;
}

void DiseaseSimulator::DrawGrid()
{
    //Background
    m_drawList->AddRectFilled(
        ImVec2(0, 0),
        m_simBoardSize,
        ImColor(0, 0, 0, 64));
    ImGui::GetForegroundDrawList()->AddRect(
        ImVec2(0, 0),
        m_simBoardSize,
        ImColor(255, 255, 255, 255));
}

void DiseaseSimulator::DrawPlots()
{
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize({ m_displaySize.x - m_toolWindowWidth, m_displaySize.y });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.f));
    ImGui::Begin("##plots", nullptr, IMGUI_PLOTWINDOW_FLAGS);
    auto Plot = [this](const char* label, const std::vector<int>& data)
    {
        ImPlot::PlotLine(label, m_data_y.data(), data.data(), (int)data.size());
    };

    if (ImPlot::BeginPlot("Entity states in every tick", ImVec2(-1.f, -1.f)))
    {
        Plot("Healthly", m_data_healthly_x);
        Plot("Infected", m_data_infected_x);
        Plot("Recovering", m_data_recovered_x);
        Plot("Sick", m_data_sick_x);
        Plot("Deaths", m_data_dead_x);
        Plot("Newborn", m_data_newborn_x);

        ImPlot::EndPlot();
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

void DiseaseSimulator::DrawToolWindow()
{
    ImGui::SetNextWindowSize(ImVec2(m_toolWindowWidth, m_displaySize.y));
    ImGui::SetNextWindowPos(ImVec2(m_displaySize.x - m_toolWindowWidth, 0));
    ImGui::Begin("##Sim", nullptr, IMGUI_TOOLWINDOW_FLAGS);
    {
        const auto offset = ImGui::GetStyle().ItemSpacing.x;
        static const std::string btnText[2] = { "Run simulation", "Stop simulation" };

        ImGui::PushStyleColor(ImGuiCol_Text, run ? ImVec4(1.f, 0.2f, 0.2f, 1.f) : ImVec4(0.2f, 1.f, 0.2f, 1.f));
        if (ImGui::Button(
            run ? btnText[1].c_str() : btnText[0].c_str(),
            ImVec2(ImGui::GetWindowWidth() - offset * 2.f, 0.f)))
            run = !run;
        ImGui::PopStyleColor();

        if (ImGui::Button("Reset simulation", ImVec2(ImGui::GetWindowWidth() - offset * 2.f, 0.f)))
            ResetSimulation();

        ImGui::Separator();
        static int roundTime = m_roundTime.count();

        ImGui::SliderInt("Max rounds", &m_numRounds, 100, 500);
        ImGui::SliderInt("Round time", &roundTime, 1, 100, "%dms");
        ImGui::SliderInt("Born chance", &m_bornChance, 0, 100, "%d%%");

        m_roundTime = std::chrono::milliseconds(uint32_t(roundTime));
        ImGui::Separator();

        auto spinnerPos = ImGui::GetCursorPos();

        ImGui::Text("Simulation initial parameters:");
        ImGui::Text("Entities to spawn: %d", m_numEntities);
        ImGui::Text("Max rounds: %d", m_numRounds);
        
        auto backup = ImGui::GetCursorPos();

        if (run)
        {
            ImGui::SetCursorPos({ m_toolWindowWidth - 60.f - 10, spinnerPos.y + (10 / 2) + 2});
            ImGui::Spinner("##performin_sim", 30, 10, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
            ImGui::SetCursorPos(backup);
        }

        ImGui::Separator();

        const float entity_perc = float(m_entities.size()) / float(MAX_ENTITIES);

        const float round_perc = float(m_curRound) / float(m_numRounds);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
        const ImU32 ent_perc_col = ImGui::GetColorU32(ImVec4(entity_perc, 1.f - entity_perc, 0.f, 1.f));
        const ImU32 round_perc_col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);

        ImGui::Text("Current simulation state:");
        ImGui::Text("%d / %d - Round", m_curRound, m_numRounds);  
        ImGui::BufferingBar("##round_prog_bar", round_perc, ImVec2(m_toolWindowWidth, 6), bg, round_perc_col);

        ImGui::Text("%d - Entities on board", m_entities.size());
        ImGui::BufferingBar("##entity_size_prog_bar", entity_perc, ImVec2(m_toolWindowWidth, 6), bg, ent_perc_col);
    }
    ImGui::End();
}

void DiseaseSimulator::DrawEntity(const Entity& ent)
{
    m_drawList->AddCircleFilled(
        ent.m_pos,
        m_entityDrawRadius,
        GetEntityColor(ent.m_state));

    /*char buff[64];
        sprintf_s(buff, "%.1f", ent.m_immuneLevel);
        ImGui::GetForegroundDrawList()->AddText(ent.m_pos, ImColor(255, 255, 255), buff);*/
}
