#include "tcxNdiFinder.h"
#include "tcxNdiInit.h"

#include <cstddef>   // NDI SDK on Arch needs NULL via this
#include <Processing.NDI.Lib.h>

namespace tcx {

namespace {
    inline NDIlib_find_instance_t cast(void* p) {
        return static_cast<NDIlib_find_instance_t>(p);
    }
}

NdiFinder::NdiFinder() {
    if (!ndi_internal::ensureInitialized()) return;

    NDIlib_find_create_t desc{};
    desc.show_local_sources = true;   // include sources on this machine
    impl_ = NDIlib_find_create_v2(&desc);
}

NdiFinder::~NdiFinder() {
    if (impl_) {
        NDIlib_find_destroy(cast(impl_));
        impl_ = nullptr;
    }
}

bool NdiFinder::refresh(int timeoutMs) {
    if (!impl_) return false;
    return NDIlib_find_wait_for_sources(cast(impl_),
                                        (uint32_t)timeoutMs);
}

std::vector<NdiSource> NdiFinder::getSources() const {
    std::vector<NdiSource> out;
    if (!impl_) return out;

    uint32_t n = 0;
    const NDIlib_source_t* arr =
        NDIlib_find_get_current_sources(cast(impl_), &n);
    if (!arr || n == 0) return out;

    out.reserve(n);
    for (uint32_t i = 0; i < n; ++i) {
        NdiSource s;
        s.name = arr[i].p_ndi_name ? arr[i].p_ndi_name : "";
        s.url  = arr[i].p_url_address ? arr[i].p_url_address : "";
        out.push_back(std::move(s));
    }
    return out;
}

} // namespace tcx
