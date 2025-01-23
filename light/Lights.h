#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <unordered_map>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLight(LightType type, const LightState& state) override;
    ndk::ScopedAStatus getSupportedTypes(std::vector<LightType>* types) override;

  private:
    void setLightBacklight(const LightState& state);
    void setLightNotification(const LightState& state);
    void applyNotificationState(const LightState& state);

    uint32_t max_led_brightness_;
    uint32_t max_screen_brightness_;

    std::unordered_map<LightType, LightState> notif_states_;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
