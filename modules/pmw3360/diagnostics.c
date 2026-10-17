/* Temporary diagnostics: observe GPIOs without reconfiguring the key scanner. */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <drivers/ext_power.h>

LOG_MODULE_REGISTER(keyball_diag, LOG_LEVEL_INF);

#define MATRIX DT_NODELABEL(kscan0)
#define ROW(n) GPIO_DT_SPEC_GET_BY_IDX(MATRIX, row_gpios, n)
#define COL(n) GPIO_DT_SPEC_GET_BY_IDX(MATRIX, col_gpios, n)
static const struct gpio_dt_spec rows[] = {ROW(0), ROW(1), ROW(2), ROW(3)};
static const struct gpio_dt_spec cols[] = {COL(0), COL(1), COL(2), COL(3), COL(4), COL(5)};
static const struct device *const bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct device *const oled = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static void diagnostics(void *a, void *b, void *c) {
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    LOG_INF("LEFT USB DIAGNOSTIC: standalone USB, BLE off, OLED idle blanking off");
    LOG_INF("kscan ready=%d, I2C ready=%d, OLED driver ready=%d",
            device_is_ready(DEVICE_DT_GET(MATRIX)), device_is_ready(bus), device_is_ready(oled));
    for (int i = 0; i < ARRAY_SIZE(rows); i++) {
        LOG_INF("ROW%d: %s pin %u flags 0x%x", i, rows[i].port->name, rows[i].pin, rows[i].dt_flags);
    }
    for (int i = 0; i < ARRAY_SIZE(cols); i++) {
        LOG_INF("COL%d: %s pin %u flags 0x%x", i, cols[i].port->name, cols[i].pin, cols[i].dt_flags);
    }
    const struct device *power = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(zmk_ext_power_generic));
    if (device_is_ready(power)) {
        int before = ext_power_get(power);
        int enabled = ext_power_enable(power);
        LOG_INF("OLED power: before=%d enable=%d after=%d", before, enabled, ext_power_get(power));
    }
    k_msleep(300);
    int recovered = i2c_recover_bus(bus);
    LOG_INF("I2C recovery result=%d", recovered);
    /* SSD1306: initialize charge pump and force all pixels on, independent of LVGL. */
    const uint8_t init[] = {0x00,0xae,0xd5,0x80,0xa8,0x1f,0xd3,0x00,0x40,
        0x8d,0x14,0x20,0x00,0xa1,0xc8,0xda,0x02,0x81,0x7f,0xd9,0xf1,
        0xdb,0x40,0xa6,0xa5,0xaf};
    for (uint16_t addr = 0x3c; addr <= 0x3d; addr++) {
        int result = i2c_write(bus, init, sizeof(init), addr);
        LOG_INF("OLED raw ALL-PIXELS-ON address=0x%x result=%d", addr, result);
    }
    int previous[4] = {-999, -999, -999, -999};
    unsigned int cycle = 0;
    for (;;) {
        int values[4];
        bool changed = false;
        for (int i = 0; i < ARRAY_SIZE(rows); i++) {
            values[i] = gpio_is_ready_dt(&rows[i]) ? gpio_pin_get_dt(&rows[i]) : -19;
            if (values[i] != previous[i]) { changed = true; }
            previous[i] = values[i];
        }
        if (changed || cycle % 20 == 0) {
            LOG_INF("row inputs [%d,%d,%d,%d] (idle outputs active; GPIO errors are negative)",
                    values[0], values[1], values[2], values[3]);
        }
        if (cycle % 20 == 0) {
            /* SSD1306 command-control byte plus NOP: tests ACK without changing the screen. */
            const uint8_t nop[] = {0x00, 0xe3};
            int result = device_is_ready(bus) ? i2c_write(bus, nop, sizeof(nop), 0x3c) : -19;
            LOG_INF("heartbeat uptime=%lld ms, OLED 0x3c NOP result=%d (0=ACK)",
                    (long long)k_uptime_get(), result);
        }
        cycle++;
        k_msleep(500);
    }
}

K_THREAD_DEFINE(keyball_diag_thread, 1536, diagnostics, NULL, NULL, NULL, 10, 0, 5000);

