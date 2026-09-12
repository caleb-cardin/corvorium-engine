// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/unique_ptr.h"
export module OS;
export import :FileProvider;
export import :Window;
export import :WindowManager;
export import :Input;
export import :InputManager;
import Event;

namespace Corvorium::OS {

export class CRVM_API OSSubsystem {
public:
    explicit OSSubsystem(Event::GlobalEventBus* EventBus)
        : pInputManager{eastl::make_unique<InputManager>()}
        , pWindowManager{eastl::make_unique<WindowManager>(pInputManager.get(), EventBus)}
    {}

    OSSubsystem(const OSSubsystem&) = delete;
    OSSubsystem& operator=(const OSSubsystem&) = delete;
    OSSubsystem(OSSubsystem&&) noexcept = default;
    OSSubsystem& operator=(OSSubsystem&&) noexcept = default;

    ~OSSubsystem() = default;


    [[nodiscard]] InputManager* GetInputManager() const { return pInputManager.get();}
    [[nodiscard]] WindowManager* GetWindowManager() const { return pWindowManager.get();}

private:
    eastl::unique_ptr<InputManager> pInputManager{nullptr};
    eastl::unique_ptr<WindowManager> pWindowManager{nullptr};
};

}
