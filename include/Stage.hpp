#ifndef STAGE_HPP
#define STAGE_HPP

#include <memory>
#include <string>

class App;

/**
 * @class Stage
 * @brief Abstract base class for game stages/levels.
 */
class Stage {
public:
    virtual ~Stage() = default;

    /**
     * @brief Called when the stage is entered.
     */
    virtual void Enter() = 0;

    /**
     * @brief Called every frame to update the stage.
     * @param deltaSeconds Delta time in seconds.
     */
    virtual void Update(float deltaSeconds) = 0;

    /**
     * @brief Called when the stage is exited (e.g., when level is completed).
     */
    virtual void Exit() = 0;

    /**
     * @brief Check if the stage has completed.
     * @return True if stage completion condition is met.
     */
    virtual bool IsStageComplete() const = 0;

    /**
     * @brief Get the name/ID of the stage.
     */
    virtual std::string GetStageName() const = 0;

    /**
     * @brief Get the name/ID of the next stage to automatically start
     *        once this stage completes (IsStageComplete() returns true).
     *        Default returns an empty string, meaning there is no next
     *        stage and the game should truly end the stage sequence here.
     */
    virtual std::string GetNextStageName() const { return ""; }
};

#endif