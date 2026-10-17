#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zmk/display/status_screen.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

static lv_obj_t *guide_label;
static char last_text[180];
static void update_guide(lv_timer_t *timer) {
    (void)timer;
    if (!guide_label) return;
    uint8_t layer_index = zmk_keymap_highest_layer_active();
    uint8_t layer_id = zmk_keymap_layer_index_to_id(layer_index);
    const char *layer = zmk_keymap_layer_name(layer_id);
    if (!layer) layer = "?";
    int profile = zmk_ble_active_profile_index();
    const char *names[] = {"PC", "Android", "Device3", "Device4", "Device5"};
    const char *name = (profile >= 0 && profile < 5) ? names[profile] : "Device";
    const char *state = zmk_ble_active_profile_is_connected() ? "LINK" :
                        zmk_ble_active_profile_is_open() ? "PAIR" : "WAIT";
    char endpoint[ZMK_ENDPOINT_STR_LEN] = {0};
    zmk_endpoint_instance_to_str(zmk_endpoints_selected(), endpoint, sizeof(endpoint));
    char text[180];
    if (layer_id == 7) {
        snprintf(text, sizeof(text),
                 "BT%d %s | %s\nTop:1 PC  2 Android\nTop:3/4/5 Other\nMid:1 USB  2 BLE", profile + 1, state, endpoint);
    } else {
        snprintf(text, sizeof(text),
                 "BT%d %s %s\nOUT:%s  Layer:%s\nHold BS thumb: guide\nTop 2: Android", profile + 1, name, state, endpoint, layer);
    }
    if (strcmp(last_text, text)) {
        lv_label_set_text(guide_label, text);
        snprintf(last_text, sizeof(last_text), "%s", text);
    }
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
    guide_label = lv_label_create(screen);
    lv_obj_set_size(guide_label, 128, 32);
    lv_obj_set_style_text_font(guide_label, &lv_font_montserrat_8, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(guide_label, 0, LV_PART_MAIN);
    lv_label_set_long_mode(guide_label, LV_LABEL_LONG_CLIP);
    lv_obj_align(guide_label, LV_ALIGN_TOP_LEFT, 0, 0);
    last_text[0] = 0;
    update_guide(NULL);
    lv_timer_create(update_guide, 200, NULL);
    return screen;
}
