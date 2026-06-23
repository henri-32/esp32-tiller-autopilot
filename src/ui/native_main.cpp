#include "diagnostics/diagnostics.h"
#include "drivers/navigationSensors.h"
#include "ui/nativeGUI.h"
#include <chrono>
#include <cstdint>
#include <types/globalTypes.h>
#include <ui/uiContent.h>

/*
Die GUI hat nichts mit dem Produktivcode zu tun, sondern ist eine Darstellungs und
Entwicklungsumgebung für das später statische UI. Deswegen alles außerhalb des
Systemcontroller. Später wird der gleiche Framebuffer vom Display Modul aufgerufen.
Deswegen werden hier Module, die zum systemController gehören einzeln instanziert,
um Darstellung mit dem produktiven FrameBuffer zu ermöglichen.
*/
DisplayConfig config; 
UIContent ui_Content{config};
nativeGUIRendering nativeRenderer;
const Intent panelIntent;
NavigationSensors::NavigationSnapshot snapshot;
DiagnosticSnapshot d_snapshot;
SystemState::SystemMode mode;

int main()
{

    const auto t0 = std::chrono::steady_clock::now();
    nativeRenderer.init();
    while (true)
    {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
        const auto loopTimestamp = static_cast<uint32_t>(elapsed);

        if (!nativeRenderer.tick(
                ui_Content.renderBuffer(panelIntent, snapshot, d_snapshot, mode, config, loopTimestamp)))
        {
            return 1;
        }
    }
    return 0;
}
