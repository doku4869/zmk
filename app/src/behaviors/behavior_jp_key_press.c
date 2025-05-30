/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_jp_key_press

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>
#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/keys.h>

// #define KC_AT                HID_USAGE_LED_UNDEFINED
// #define KC_CARET             HID_USAGE_LED_NUM_LOCK
// #define KC_AMPERSAND         HID_USAGE_LED_CAPS_LOCK
// #define KC_ASTERISK          HID_USAGE_LED_SCROLL_LOCK
// #define KC_LEFT_PARENTHESIS  HID_USAGE_LED_COMPOSE
// #define KC_RIGHT_PARENTHESIS HID_USAGE_LED_KANA
// #define KC_GRV               HID_USAGE_LED_POWER
// #define KC_MINS              HID_USAGE_LED_SHIFT
// #define KC_EQL               HID_USAGE_LED_DO_NOT_DISTURB
// #define KC_LBRC              HID_USAGE_LED_MUTE
// #define KC_RBRC              HID_USAGE_LED_TONE_ENABLE
// #define KC_BSLS              HID_USAGE_LED_HIGH_CUT_FILTER
// #define JPUS                 HID_USAGE_LED_LOW_CUT_FILTER
/*------------------------------*/
bool jpmode = true;
int jp_keymap_len = 22;

// param1, already_shift, out_keycode, needs_shift
struct behavior_binding_jp_keymap {
    uint32_t param1;
    bool already_shift;
    uint32_t out_keycode;
    bool needs_shift;
    int tap_count;
};

struct behavior_binding_jp_keymap jp_keymap[] = {
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_2_AND_AT), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE), false, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_6_AND_CARET), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS), false, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_6_AND_CARET), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE), false, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE), false, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_BACKSLASH_AND_PIPE), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL3), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_BACKSLASH_AND_PIPE), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1), false, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE), false, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_2_AND_AT), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_GRAVE_ACCENT_AND_TILDE), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_GRAVE_ACCENT_AND_TILDE), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_CAPS_LOCK), false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_CAPS_LOCK), true, 0},
    {ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_CAPS_LOCK), true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_CAPS_LOCK), false, 0},
    {AT, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE), false, 0},
    {AT, true, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE), false, 0},
    {CARET, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS), false, 0},
    {AMPERSAND, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_6_AND_CARET), true, 0},
    {ASTERISK, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE), true, 0},
    {LEFT_PARENTHESIS, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK), true, 0},
    {RIGHT_PARENTHESIS, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS), true, 0},
    {TILD, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS), true, 0},
    {UNDER, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1), true, 0},
    {PLUS, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON), true, 0},
    {LBRC, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE), true, 0},
    {RBRC, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE), true, 0},
    {PIPE, false, ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL3), true, 0}
};



/*------------------------------*/

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Key",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE,
    },
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif

static bool is_shift_active() {
    zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
    return (mods & (MOD_LSFT | MOD_RSFT)) != 0;
}

// JIS変換関数例
static void convert_jis_key(uint32_t param1, bool *needs_shift, uint32_t *out_keycode,
                            bool shift_already) {
    *needs_shift = shift_already;
    *out_keycode = param1;

    for (int i = 0; i < jp_keymap_len; i++) {
        if (jp_keymap[i].param1 == param1 && jp_keymap[i].already_shift == shift_already) {
            *out_keycode = jp_keymap[i].out_keycode;
            *needs_shift = jp_keymap[i].needs_shift;
            jp_keymap[i].tap_count++;
            return;
        }
    }
}

// JIS変換関数例
static void convert_jis_key_release(uint32_t param1, bool *needs_shift, uint32_t *out_keycode,
                            bool shift_already) {
    *needs_shift = shift_already;
    *out_keycode = param1;

    for (int i = 0; i < jp_keymap_len; i++) {
        if (jp_keymap[i].param1 == param1 && jp_keymap[i].tap_count > 0) {
            jp_keymap[i].tap_count--;
            *out_keycode = jp_keymap[i].out_keycode;
            *needs_shift = jp_keymap[i].already_shift;
            return;
        }
    }
}

static void shift_encoded_key(bool needs_shift, bool shift_already) {
    if (needs_shift && !shift_already) {
        raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT, true,
                                                        k_uptime_get());
    } else if (!needs_shift && shift_already) {
        raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT, false,
                                                        k_uptime_get());
    }
    return;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding * binding,
                                        struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;
    bool needs_shift = false;

    bool shift_already = is_shift_active();
    // if(binding->param1 == JPUS) {
    //     jpmode = !jpmode;
    // }

    if (jpmode) {
        convert_jis_key(binding->param1, &needs_shift, &keycode, shift_already);
    }

    shift_encoded_key(needs_shift, shift_already);

    LOG_DBG("position %d keycode 0x%02X", event.position, keycode);
    int ret = raise_zmk_keycode_state_changed_from_encoded(keycode, true, event.timestamp);

    return ret;
}

static int on_keymap_binding_released(struct zmk_behavior_binding * binding,
                                        struct zmk_behavior_binding_event event) {
    uint32_t keycode = binding->param1;
    bool needs_shift = false;

    bool shift_already = is_shift_active();

    if (jpmode) {
        convert_jis_key_release(binding->param1, &needs_shift, &keycode, shift_already);
    }

    int ret = raise_zmk_keycode_state_changed_from_encoded(keycode, false, event.timestamp);

    shift_encoded_key(needs_shift, shift_already);

    LOG_DBG("position %d keycode 0x%02X", event.position, keycode);
    return ret;
}

static const struct behavior_driver_api behavior_key_press_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

#define KP_INST(n)                                                                                 \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_key_press_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)
