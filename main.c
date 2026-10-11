#include <string.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "hardware/regs/io_qspi.h"
#include "tusb.h"

static bool __no_inline_not_in_flash_func(bootsel_pressed)(void) {
    uint32_t flags = save_and_disable_interrupts();

    hw_write_masked(&ioqspi_hw->io[1].ctrl,
        GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
        IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    for (volatile int i = 0; i < 1000; ++i) {}

    bool pressed = !(sio_hw->gpio_hi_in & (1u << 1));

    hw_write_masked(&ioqspi_hw->io[1].ctrl,
        GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
        IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);
    return pressed;
}

uint8_t const hid_report_desc[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return hid_report_desc;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
    hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
    hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)bufsize;
}

tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0xCafe,
    .idProduct = 0x4001,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

uint8_t const desc_config[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0,
        TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD,
        sizeof(hid_report_desc), 0x81, 8, 10)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_config;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t desc[32];
    const char *strings[] = {"", "Pico", "BOOTSEL Keyboard", "0001"};
    if (index >= 4) return NULL;

    uint8_t count = 0;
    if (index == 0) {
        desc[1] = 0x0409;
        count = 1;
    } else {
        count = (uint8_t)strlen(strings[index]);
        if (count > 31) count = 31;
        for (uint8_t i = 0; i < count; ++i)
            desc[1 + i] = strings[index][i];
    }

    desc[0] = (TUSB_DESC_STRING << 8) | (2 * count + 2);
    return desc;
}

int main(void) {
    tusb_init();
    bool previous = false;
    uint8_t keys[6] = {0};

    while (true) {
        tud_task();
        bool pressed = bootsel_pressed();

        if (tud_hid_ready() && pressed != previous) {
            memset(keys, 0, sizeof(keys));
            if (pressed) keys[0] = HID_KEY_SPACE;
            if (tud_hid_keyboard_report(0, 0, keys))
                previous = pressed;
        }

        sleep_ms(2);
    }
}
