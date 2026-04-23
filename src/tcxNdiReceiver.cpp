#include "tcxNdiReceiver.h"
#include "tcxNdiInit.h"

#include <cstddef>   // NDI SDK on Arch needs NULL via this
#include <cstring>
#include <Processing.NDI.Lib.h>

#include "tc/utils/tcLog.h"

namespace tcx {

namespace {
    inline NDIlib_recv_instance_t cast(void* p) {
        return static_cast<NDIlib_recv_instance_t>(p);
    }
}

NdiReceiver::NdiReceiver() = default;

NdiReceiver::~NdiReceiver() {
    disconnect();
}

bool NdiReceiver::connect(const NdiSource& source) {
    if (!ndi_internal::ensureInitialized()) return false;
    disconnect();

    NDIlib_source_t src{};
    src.p_ndi_name    = source.name.c_str();
    src.p_url_address = source.url.empty() ? nullptr : source.url.c_str();

    NDIlib_recv_create_v3_t desc{};
    desc.source_to_connect_to = src;
    // Ask NDI to convert to RGBA on the receive side so we can hand the
    // buffer straight to tc::Pixels without an extra colorspace pass.
    desc.color_format = NDIlib_recv_color_format_RGBX_RGBA;
    desc.bandwidth    = NDIlib_recv_bandwidth_highest;
    desc.allow_video_fields = false;

    NDIlib_recv_instance_t r = NDIlib_recv_create_v3(&desc);
    if (!r) {
        trussc::logError("tcxNdi") << "NdiReceiver: NDIlib_recv_create_v3 failed";
        return false;
    }
    impl_     = r;
    hasFrame_ = false;
    trussc::logNotice("tcxNdi") << "NdiReceiver connected: '"
                                << source.name << "'";
    return true;
}

void NdiReceiver::disconnect() {
    if (impl_) {
        NDIlib_recv_destroy(cast(impl_));
        impl_ = nullptr;
    }
    hasFrame_ = false;
}

bool NdiReceiver::update(int timeoutMs) {
    if (!impl_) return false;

    NDIlib_video_frame_v2_t v{};
    NDIlib_audio_frame_v3_t a{};
    NDIlib_metadata_frame_t m{};

    auto type = NDIlib_recv_capture_v3(cast(impl_), &v, &a, &m, (uint32_t)timeoutMs);

    bool gotVideo = false;
    if (type == NDIlib_frame_type_video) {
        if (v.xres > 0 && v.yres > 0 && v.p_data) {
            // The receiver was created with RGBX/RGBA — but NDI may hand us
            // RGBA, RGBX, or BGRA depending on negotiation. RGBA/RGBX are
            // byte-identical from a tc::Pixels perspective; BGRA we swizzle
            // here. Anything else falls through with a warning.
            const bool isFourByte =
                v.FourCC == NDIlib_FourCC_video_type_RGBA ||
                v.FourCC == NDIlib_FourCC_video_type_RGBX ||
                v.FourCC == NDIlib_FourCC_video_type_BGRA ||
                v.FourCC == NDIlib_FourCC_video_type_BGRX;

            if (isFourByte) {
                if (pixels_.getWidth()    != v.xres ||
                    pixels_.getHeight()   != v.yres ||
                    pixels_.getChannels() != 4) {
                    pixels_.allocate(v.xres, v.yres, 4);
                }

                const int rowBytes = v.xres * 4;
                uint8_t* dst       = pixels_.getData();
                const uint8_t* src = v.p_data;
                if (v.FourCC == NDIlib_FourCC_video_type_BGRA ||
                    v.FourCC == NDIlib_FourCC_video_type_BGRX)
                {
                    // swap B<->R per pixel
                    for (int y = 0; y < v.yres; ++y) {
                        const uint8_t* s = src + y * v.line_stride_in_bytes;
                        uint8_t*       d = dst + y * rowBytes;
                        for (int x = 0; x < v.xres; ++x) {
                            d[0] = s[2]; d[1] = s[1]; d[2] = s[0]; d[3] = s[3];
                            s += 4; d += 4;
                        }
                    }
                } else {
                    for (int y = 0; y < v.yres; ++y) {
                        std::memcpy(dst + y * rowBytes,
                                    src + y * v.line_stride_in_bytes,
                                    rowBytes);
                    }
                }
                hasFrame_ = true;
                gotVideo  = true;
            } else {
                trussc::logWarning("tcxNdi")
                    << "NdiReceiver: unhandled FourCC 0x"
                    << std::hex << (uint32_t)v.FourCC;
            }
        }
        NDIlib_recv_free_video_v2(cast(impl_), &v);
    } else if (type == NDIlib_frame_type_audio) {
        NDIlib_recv_free_audio_v3(cast(impl_), &a);
    } else if (type == NDIlib_frame_type_metadata) {
        NDIlib_recv_free_metadata(cast(impl_), &m);
    }

    return gotVideo;
}

} // namespace tcx
