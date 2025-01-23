#include <aidl/android/hardware/light/BnLight.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "Lights.h"

using aidl::android::hardware::light::Light;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Light> light = ndk::SharedRefBase::make<Light>();

    const std::string instance = std::string() + Light::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(light->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE; // Should never reach
}
