#include "tcxNdiInit.h"

#include <Processing.NDI.Lib.h>
#include <atomic>
#include <cstdlib>
#include <mutex>

#include "tc/utils/tcLog.h"

namespace tcx {
namespace ndi_internal {

namespace {
    std::once_flag g_initFlag;
    std::atomic<bool> g_ok{false};

    void runInit() {
        if (!NDIlib_initialize()) {
            trussc::logError("tcxNdi") << "NDIlib_initialize() failed — "
                "this CPU may not be supported by the NDI runtime.";
            return;
        }
        std::atexit([]() { NDIlib_destroy(); });
        g_ok = true;
        trussc::logNotice("tcxNdi") << "NDI runtime initialized (v"
            << NDIlib_version() << ")";
    }
}

bool ensureInitialized() {
    std::call_once(g_initFlag, runInit);
    return g_ok.load();
}

} // namespace ndi_internal
} // namespace tcx
