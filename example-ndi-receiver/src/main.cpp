#include "TrussC.h"
#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.title   = "NDI Receiver Example";
    settings.width   = 960;
    settings.height  = 540;
    settings.highDpi = false;
    return TC_RUN_APP(tcApp, settings);
}
