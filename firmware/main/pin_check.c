#include "pin_check.h"

// GPIO22-25 do not exist on the ESP32-S3 (SOC_GPIO_VALID_GPIO_MASK).
static bool pin_exists(int gpio)
{
    return gpio >= 0 && gpio <= 48 && !(gpio >= 22 && gpio <= 25);
}

static bool pin_reserved(int gpio)
{
    return gpio == 19 || gpio == 20 || (gpio >= 26 && gpio <= 32);
}

pin_status_t pin_check(int gpio, const int *configured, size_t count)
{
    if (gpio == -1) {
        return PIN_DISABLED;
    }
    if (!pin_exists(gpio)) {
        return PIN_ERR_INVALID;
    }
    if (pin_reserved(gpio)) {
        return PIN_ERR_RESERVED;
    }
    size_t uses = 0;
    for (size_t i = 0; i < count; ++i) {
        if (configured[i] == gpio) {
            ++uses;
        }
    }
    return uses > 1 ? PIN_ERR_DUPLICATE : PIN_OK;
}

bool pin_is_strapping(int gpio)
{
    return gpio == 0 || gpio == 3 || gpio == 45 || gpio == 46;
}
