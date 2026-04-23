#include "tcxNdiSender.h"
#include "tcxNdiInit.h"

#include <Processing.NDI.Lib.h>

#include "tc/utils/tcLog.h"

namespace tcx {

namespace {
    inline NDIlib_send_instance_t cast(void* p) {
        return static_cast<NDIlib_send_instance_t>(p);
    }
}

NdiSender::NdiSender() = default;

NdiSender::~NdiSender() {
    close();
}

bool NdiSender::setup(const std::string& name) {
    if (!ndi_internal::ensureInitialized()) return false;
    close();

    NDIlib_send_create_t desc{};
    desc.p_ndi_name = name.c_str();
    desc.clock_video = false;   // app drives the cadence

    NDIlib_send_instance_t s = NDIlib_send_create(&desc);
    if (!s) {
        trussc::logError("tcxNdi") << "NdiSender: NDIlib_send_create('"
                                   << name << "') failed";
        return false;
    }
    impl_ = s;
    name_ = name;
    trussc::logNotice("tcxNdi") << "NdiSender ready: '" << name << "'";
    return true;
}

void NdiSender::close() {
    if (impl_) {
        NDIlib_send_destroy(cast(impl_));
        impl_ = nullptr;
    }
    name_.clear();
}

bool NdiSender::send(const trussc::Pixels& pixels) {
    if (!impl_) return false;
    if (pixels.getWidth() <= 0 || pixels.getHeight() <= 0) return false;

    NDIlib_FourCC_video_type_e fourcc;
    int stride;
    switch (pixels.getChannels()) {
        case 4:
            // tc::Pixels stores RGBA; NDI accepts RGBA directly (NDIlib v5+).
            fourcc = NDIlib_FourCC_type_RGBA;
            stride = pixels.getWidth() * 4;
            break;
        case 3:
            // NDI doesn't have a packed-RGB-without-alpha FourCC; reject so
            // callers know to convert. Most TrussC capture sources are RGBA
            // anyway.
            trussc::logWarning("tcxNdi")
                << "NdiSender: 3-channel (RGB) frames not supported; "
                   "pad to RGBA before send().";
            return false;
        default:
            trussc::logWarning("tcxNdi")
                << "NdiSender: unsupported channel count "
                << pixels.getChannels();
            return false;
    }

    NDIlib_video_frame_v2_t frame{};
    frame.xres   = pixels.getWidth();
    frame.yres   = pixels.getHeight();
    frame.FourCC = fourcc;
    frame.line_stride_in_bytes = stride;
    frame.p_data = const_cast<uint8_t*>(pixels.getData());

    NDIlib_send_send_video_v2(cast(impl_), &frame);
    return true;
}

int NdiSender::getNumConnections() const {
    if (!impl_) return 0;
    // 0ms timeout — non-blocking query.
    return NDIlib_send_get_no_connections(cast(impl_), 0);
}

} // namespace tcx
