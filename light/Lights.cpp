#include "Lights.h"
#include <android-base/file.h>
#include <android-base/logging.h>

#define LOG_TAG "LightsService"

using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;

namespace aidl {
namespace android {
namespace hardware {
namespace light {

// SysFS paths (same as original)
#define LCD_ATTR(x) "/sys/class/leds/lcd-backlight/" #x
#define WHITE_ATTR(x) "/sys/class/leds/red/" #x

namespace {

constexpr auto kDefaultMaxLedBrightness = 255;
constexpr auto kDefaultMaxScreenBrightness = 4095;

bool WriteToFile(const std::string& path, uint32_t content) {
    return WriteStringToFile(std::to_string(content), path);
}

uint32_t RgbaToBrightness(uint32_t color, uint32_t max_brightness) {
    uint32_t alpha = (color >> 24) & 0xFF;
    uint32_t red = (color >> 16) & 0xFF;
    uint32_t green = (color >> 8) & 0xFF;
    uint32_t blue = color & 0xFF;

    if (alpha != 0xFF) {
        red = red * alpha / 0xFF;
        green = green * alpha / 0xFF;
        blue = blue * alpha / 0xFF;
    }

    return (77 * red + 150 * green + 29 * blue) * max_brightness / 0xFF / 0xFF;
}

bool IsLit(uint32_t color) {
    return color & 0x00FFFFFF;
}

}  // namespace

Lights::Lights() {
    std::string buf;

    if (ReadFileToString(LCD_ATTR(max_brightness), &buf)) {
        max_screen_brightness_ = std::stoi(buf);
    } else {
        max_screen_brightness_ = kDefaultMaxScreenBrightness;
        LOG(ERROR) << "Failed to read max screen brightness";
    }

    if (ReadFileToString(WHITE_ATTR(max_brightness), &buf)) {
        max_led_brightness_ = std::stoi(buf);
    } else {
        max_led_brightness_ = kDefaultMaxLedBrightness;
        LOG(ERROR) << "Failed to read max LED brightness";
    }
}

ndk::ScopedAStatus Lights::setLight(LightType type, const LightState& state) {
    switch(type) {
        case LightType::BACKLIGHT:
            setLightBacklight(state);
            break;
        case LightType::BATTERY:
        case LightType::NOTIFICATIONS:
            notif_states_[type] = state;
            applyNotificationState(state);
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getSupportedTypes(std::vector<LightType>* types) {
    types->push_back(LightType::BACKLIGHT);
    types->push_back(LightType::BATTERY);
    types->push_back(LightType::NOTIFICATIONS);
    return ndk::ScopedAStatus::ok();
}

void Lights::setLightBacklight(const LightState& state) {
    uint32_t brightness = RgbaToBrightness(state.color, max_screen_brightness_);
    WriteToFile(LCD_ATTR(brightness), brightness);
}

void Lights::applyNotificationState(const LightState& state) {
    uint32_t white_brightness = RgbaToBrightness(state.color, max_led_brightness_);

    WriteToFile(WHITE_ATTR(breath), 0); // Turn off initially

    if (state.flashMode == FlashMode::TIMED && state.flashOnMs > 0 && state.flashOffMs > 0) {
        WriteToFile(WHITE_ATTR(delay_off), static_cast<uint32_t>(state.flashOffMs));
        WriteToFile(WHITE_ATTR(delay_on), static_cast<uint32_t>(state.flashOnMs));
        WriteToFile(WHITE_ATTR(breath), 1);
    } else {
        WriteToFile(WHITE_ATTR(brightness), white_brightness);
    }
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
