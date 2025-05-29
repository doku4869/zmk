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

/*------------------------------*/
bool jpmode = true;
int jp_keymap_len = 22;

// param1, already_shift, out_keycode, needs_shift
struct behavior_binding_jp_keymap {
    uint8_t param1;
    bool already_shift;
    uint8_t out_keycode;
    bool needs_shift;
    int tap_count;
};

struct behavior_binding_jp_keymap jp_keymap[] = {
{HID_USAGE_KEY_KEYBOARD_2_AND_AT, true, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE, false, 0},
{HID_USAGE_KEY_KEYBOARD_6_AND_CARET , true, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS, false, 0},
{HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND , true, HID_USAGE_KEY_KEYBOARD_6_AND_CARET, true, 0},
{HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK , true, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE, true, 0},
{HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS , true, HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK, true, 0},
{HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS , true, HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS, true, 0},
{HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE , true, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1, true, 0},
{HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS , false, HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE, true, 0},
{HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS , true, HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON, true, 0},
{HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE , true, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE, true, 0},
{HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE , false, HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE, false, 0},
{HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE , true, HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE, true, 0},
{HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE , false, HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE, false, 0},
{HID_USAGE_KEY_KEYBOARD_BACKSLASH_AND_PIPE , true, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL3, true, 0},
{HID_USAGE_KEY_KEYBOARD_BACKSLASH_AND_PIPE , false, HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1, false, 0},
{HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON , true, HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE, false, 0},
{HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE , true, HID_USAGE_KEY_KEYBOARD_2_AND_AT, true, 0},
{HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE , false, HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND, true, 0},
{HID_USAGE_KEY_KEYBOARD_GRAVE_ACCENT_AND_TILDE , true, HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS, true, 0},
{HID_USAGE_KEY_KEYBOARD_GRAVE_ACCENT_AND_TILDE , false, HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE, true, 0},
{HID_USAGE_KEY_KEYBOARD_CAPS_LOCK , false, HID_USAGE_KEY_KEYBOARD_CAPS_LOCK, true, 0},
{HID_USAGE_KEY_KEYBOARD_CAPS_LOCK , true, HID_USAGE_KEY_KEYBOARD_CAPS_LOCK, false, 0}
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
static void convert_jis_key(uint8_t param1, bool *needs_shift, uint8_t *out_keycode,
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
static void convert_jis_key_release(uint8_t param1, bool *needs_shift, uint8_t *out_keycode,
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

static int on_keymap_binding_pressed(struct zmk_behavior_binding * binding,
                                        struct zmk_behavior_binding_event event) {
    uint8_t keycode = binding->param1;
    bool needs_shift = false;

    bool shift_already = is_shift_active();

    if (jpmode) {
        convert_jis_key(binding->param1, &needs_shift, &keycode, shift_already);
    }

    if (needs_shift && !shift_already) {
        raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT, true,
                                                        event.timestamp);
    } else if (!needs_shift && shift_already) {
        raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT, false,
                                                        event.timestamp);
    }

    LOG_DBG("position %d keycode 0x%02X", event.position, keycode);
    int ret = raise_zmk_keycode_state_changed_from_encoded(keycode, true, event.timestamp);

    return ret;
}

static int on_keymap_binding_released(struct zmk_behavior_binding * binding,
                                        struct zmk_behavior_binding_event event) {
    uint8_t keycode = binding->param1;
    bool needs_shift = false;

    bool shift_already = is_shift_active();

    if (jpmode) {
        convert_jis_key_release(binding->param1, &needs_shift, &keycode, shift_already);
    }

    int ret = raise_zmk_keycode_state_changed_from_encoded(keycode, false, event.timestamp);

    if (needs_shift && !shift_already) {
        raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT, false,
                                                        event.timestamp);
    } else if (!needs_shift && shift_already) {
        raise_zmk_keycode_state_changed_from_encoded(HID_USAGE_KEY_KEYBOARD_LEFTSHIFT, true,
                                                        event.timestamp);
    }

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
