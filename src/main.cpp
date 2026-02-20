#include "app/Application.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // Allocate console for debug output in development
    #ifdef _DEBUG
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    #endif
#else
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
#endif

    auto& app = ComfyX::Application::instance();

    if (!app.initialize()) {
        std::cerr << "[ComfyX] Failed to initialize application" << std::endl;
#ifdef _WIN32
        MessageBoxA(nullptr, "Failed to initialize ComfyX.\nCheck the console for details.",
                     "ComfyX Error", MB_OK | MB_ICONERROR);
#endif
        return 1;
    }

    app.run();
    app.shutdown();

    return 0;
}
