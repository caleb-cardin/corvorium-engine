// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module Rendering:Context;
export import :Device;
export namespace Corvorium::Rendering {

class RenderDevice;

class RenderContext {
public:
    virtual ~RenderContext() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

};

}
