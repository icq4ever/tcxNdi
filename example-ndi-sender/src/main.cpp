#include "TrussC.h"
#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.title   = "NDI Sender Example";
    settings.width   = 640;
    settings.height  = 360;
    settings.highDpi = false;
    return TC_RUN_APP(tcApp, settings);
}
