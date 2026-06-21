#ifndef STAGE_MANAGER_HPP
#define STAGE_MANAGER_HPP

#include <memory>
#include <string>

class App;
class Stage;

/**
 * @class StageManager
 * @brief Manages stage lifecycle and transitions.
 */
class StageManager {
public:
    explicit StageManager(App* app);
    ~StageManager() = default;

    /**
     * @brief Start a stage by name.
     * @param stageName The name of the stage to start (e.g., "1-1").
     */
    void StartStage(const std::string& stageName);

    /**
     * @brief Stop the currently active stage (calls Exit() and clears it).
     *        Safe to call even if no stage is active.
     */
    void StopStage();

    /**
     * @brief Update the current stage.
     * @param deltaSeconds Delta time in seconds.
     */
    void Update(float deltaSeconds);

    /**
     * @brief Check if a stage is currently active.
     */
    bool HasActiveStage() const;

    /**
     * @brief Get the name of the currently active stage.
     */
    std::string GetCurrentStageName() const;

private:
    App* m_App;
    std::shared_ptr<Stage> m_CurrentStage;
};

#endif