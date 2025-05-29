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

    if (param1 == HID_USAGE_KEY_KEYBOARD_2_AND_AT && shift_already == true) { // @/Shift + 2
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE; // [
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_6_AND_CARET &&
               shift_already == true) { // ^/Shift + 6
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS; // Shift + =
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND &&
               shift_already == true) { // &/Shift + 7
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_6_AND_CARET; // Shift + 6
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK &&
               shift_already == true) { // */Shift + 8
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE; // Shift + '
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS &&
               shift_already == true) { // (/Shift + 9
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK; // Shift + 8
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS &&
               shift_already == true) { // )/Shift + 0
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS; // Shift + 9
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE &&
               shift_already == true) { // _/Shift + -
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1; // Shift + ¥
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS && shift_already == false) { // =
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE; // Shift + -
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS &&
               shift_already == true) { // +/Shift + =
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON; // Shift + ;
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE &&
               shift_already == true) { // {/Shift + [
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE; // Shift + ]
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE &&
               shift_already == false) { // [
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE; // ]
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE &&
               shift_already == true) { // }/Shift + ]
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE; // Shift + ~
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_BRACE &&
               shift_already == false) { // ]
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_NON_US_HASH_AND_TILDE; // ~
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_BACKSLASH_AND_PIPE &&
               shift_already == true) {                       // |/Shift + \ 
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_INTERNATIONAL3; // Shift + ¥
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_BACKSLASH_AND_PIPE &&
               shift_already == false) {                      // |/Shift + \ 
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_INTERNATIONAL1; // \ 
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON && shift_already == true) { // :/Shift + ;
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE; // '
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE && shift_already == true) { // "/Shift + '
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_2_AND_AT; // Shift + 2
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_APOSTROPHE_AND_QUOTE && shift_already == false) { // '
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND; // Shift + 2
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_GRAVE_ACCENT_AND_TILDE && shift_already == true) { // '
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_EQUAL_AND_PLUS; // Shift + 2
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_GRAVE_ACCENT_AND_TILDE && shift_already == false) { // '
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_LEFT_BRACKET_AND_LEFT_BRACE; // Shift + 2
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_CAPS_LOCK && shift_already == false) { // '
        *needs_shift = true;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_CAPS_LOCK; // Shift + 2
    } else if (param1 == HID_USAGE_KEY_KEYBOARD_CAPS_LOCK && shift_already == true) { // '
        *needs_shift = false;
        *out_keycode = HID_USAGE_KEY_KEYBOARD_CAPS_LOCK; // Shift + 2
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
        convert_jis_key(binding->param1, &needs_shift, &keycode, shift_already);
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
