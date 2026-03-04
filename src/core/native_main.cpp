#include "ui/nativeGUI.h"
#include <chrono>
#include <cstdint>
#include <ui/uiContent.h>

/*
Die GUI hat nichts mit dem Produktivcode zu tun, sondern ist eine Darstellungs und
Entwicklungsumgebung für das später statische UI. Deswegen alles außerhalb des
Systemcontroller. Später wird der gleiche Framebuffer vom Display Modul aufgerufen.
Deswegen werden hier Module, die zum systemController gehören einzeln instanziert,
um Darstellung mit dem produktiven FrameBuffer zu ermöglichen.
*/

UIContent ui_Content{600, 600};
nativeGUIRendering nativeRenderer; 
int main()
{

    const auto t0 = std::chrono::steady_clock::now();

    while (true)
    {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
        const auto loopTimestamp = static_cast<uint32_t>(elapsed);
		
		nativeRenderer.tick();
    }
}
