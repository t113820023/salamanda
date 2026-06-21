#include "StageManager.hpp"
#include "Stage.hpp"
#include "stages/Stage1_1.hpp"
#include "stages/Stage1_2.hpp"
#include "App.hpp"
#include "Util/Logger.hpp"

StageManager::StageManager(App* app) 
    : m_App(app), m_CurrentStage(nullptr) {
}

void StageManager::StartStage(const std::string& stageName) {
    // Exit current stage if any
    StopStage();

    // Create and enter new stage
    if (stageName == "1-1") {
        m_CurrentStage = std::make_shared<Stage1_1>(m_App);
    } else if (stageName == "1-2") {
        m_CurrentStage = std::make_shared<Stage1_2>(m_App);
    } else {
        LOG_ERROR("Unknown stage: {}", stageName);
        return;
    }

    if (m_CurrentStage) {
        LOG_INFO("Starting stage: {}", stageName);
        m_CurrentStage->Enter();
    }
}

void StageManager::StopStage() {
    if (m_CurrentStage) {
        m_CurrentStage->Exit();
        m_CurrentStage = nullptr;
    }
}

void StageManager::Update(float deltaSeconds) {
    if (!m_CurrentStage) {
        return;
    }

    m_CurrentStage->Update(deltaSeconds);

    if (m_CurrentStage->IsStageComplete()) {
        LOG_INFO("Stage {} completed!", m_CurrentStage->GetStageName());

        const std::string nextStageName = m_CurrentStage->GetNextStageName();
        m_CurrentStage->Exit();
        m_CurrentStage = nullptr;

        if (!nextStageName.empty()) {
            LOG_INFO("Auto-advancing to next stage: {}", nextStageName);
            StartStage(nextStageName);
        }
    }
}

bool StageManager::HasActiveStage() const {
    return m_CurrentStage != nullptr;
}

std::string StageManager::GetCurrentStageName() const {
    if (m_CurrentStage) {
        return m_CurrentStage->GetStageName();
    }
    return "";
}