// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
export module Rendering:Device;

export namespace Corvorium::Rendering {
class RenderContext;

class CRVM_API RenderDevice {
public:
    virtual ~RenderDevice() = default;

    [[nodiscard]] virtual bool IsInitialized() const = 0;
};
}
