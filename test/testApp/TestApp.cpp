// Copyright © 2026 Caleb Cardin. All rights reserved.

import Application;

class TestApp : public Corvorium::Application {
public:
    TestApp()
        : Application{"Test App"}
    {}

private:
    void SomethingElse() override
    {}
};

int main(int argc, const char** argv)
{
    TestApp app{};

    if (app.IsInitialized())
    {
        app.Run();
        return 0;
    }
    return 1;
}
