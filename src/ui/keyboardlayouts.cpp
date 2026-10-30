#include "ui/keyboardlayouts.h"

#include <QChar>
#include <QGuiApplication>
#include <QLatin1String>
#include <QString>

#include "core/keys.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon.h>

// --- Version ------------------------------------------------------------
// src/ui/keyboardlayouts.cpp 0.3.10

// --- Version history ----------------------------------------------------
// v0.3.10: Initial: which keyboard layout a key comes from, Latin or not

namespace chrono_calc {

namespace {

struct ContextDeleter
{
    void operator()(xkb_context* context) const { xkb_context_unref(context); }
};

struct KeymapDeleter
{
    void operator()(xkb_keymap* keymap) const { xkb_keymap_unref(keymap); }
};

using ContextPointer = std::unique_ptr<xkb_context, ContextDeleter>;
using KeymapPointer = std::unique_ptr<xkb_keymap, KeymapDeleter>;

// The XKB key codes of Linux are the PC scan codes plus 8
constexpr std::uint32_t xkbOffset = 8;
constexpr std::uint32_t lastLetterScanCode = 0x32;

// XKB names: rules, model, layouts, variants and options; an empty rules or model is XKB's default (evdev, pc105)
struct LayoutNames
{
    std::string rules;
    std::string model;
    std::string layouts;
    std::string variants;
    std::string options;
};

KeymapPointer keymapFromNames(xkb_context* context, const LayoutNames& given)
{
    const xkb_rule_names names{.rules = given.rules.empty() ? nullptr : given.rules.c_str(),
                               .model = given.model.empty() ? nullptr : given.model.c_str(),
                               .layout = given.layouts.c_str(), .variant = given.variants.c_str(),
                               .options = given.options.c_str()};
    return KeymapPointer(xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS));
}

// X11: the names the desktop gave the X server, in the root window's _XKB_RULES_NAMES: rules, model, layouts,
// variants and options, each ended by a zero
KeymapPointer keymapOfX11(xkb_context* context)
{
    xcb_connection_t* connection = xcb_connect(nullptr, nullptr);
    const auto disconnect = [connection] { xcb_disconnect(connection); };
    if (xcb_connection_has_error(connection) != 0) {
        disconnect();
        return nullptr;
    }
    const xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    constexpr std::string_view atomName = "_XKB_RULES_NAMES";
    std::unique_ptr<xcb_intern_atom_reply_t, decltype(&std::free)> atom(
        xcb_intern_atom_reply(connection,
                              xcb_intern_atom(connection, 1, static_cast<std::uint16_t>(atomName.size()), atomName.data()),
                              nullptr),
        &std::free);
    if (screen == nullptr || !atom || atom->atom == XCB_ATOM_NONE) {
        disconnect();
        return nullptr;
    }
    constexpr std::uint32_t longest = 1024;
    const std::unique_ptr<xcb_get_property_reply_t, decltype(&std::free)> property(
        xcb_get_property_reply(connection,
                               xcb_get_property(connection, 0, screen->root, atom->atom, XCB_ATOM_STRING, 0, longest),
                               nullptr),
        &std::free);
    std::vector<std::string> parts;
    if (property) {
        const auto* text = static_cast<const char*>(xcb_get_property_value(property.get()));
        const auto length = static_cast<std::size_t>(xcb_get_property_value_length(property.get()));
        std::string part;
        for (std::size_t index = 0; index < length; ++index) {
            if (text[index] == '\0') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                parts.push_back(part);
                part.clear();
            } else {
                part += text[index];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
        }
    }
    disconnect();
    constexpr std::size_t partCount = 5;
    if (parts.size() < partCount || parts[2].empty()) {
        return nullptr;
    }
    return keymapFromNames(context, {.rules = parts[0], .model = parts[1], .layouts = parts[2], .variants = parts[3],
                                     .options = parts[4]});
}

// Wayland: the keyboard description the desktop gives every program, asked over a connection of our own
struct WaylandKeyboard
{
    xkb_context* context = nullptr;
    wl_seat* seat = nullptr;
    KeymapPointer keymap;
};

void onGlobal(void* data, wl_registry* registry, std::uint32_t name, const char* interface, std::uint32_t /*version*/)
{
    auto* keyboard = static_cast<WaylandKeyboard*>(data);
    if (keyboard->seat == nullptr && QLatin1String(interface) == QLatin1String(wl_seat_interface.name)) {
        keyboard->seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
    }
}

void onGlobalRemove(void* /*data*/, wl_registry* /*registry*/, std::uint32_t /*name*/) {}

// The parameters are the Wayland protocol's own
void onKeymap(void* data, wl_keyboard* /*keyboard*/, std::uint32_t format,  // NOLINT(bugprone-easily-swappable-parameters)
              std::int32_t descriptor, std::uint32_t size)
{
    auto* keyboard = static_cast<WaylandKeyboard*>(data);
    if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 && size > 0) {
        void* text = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, descriptor, 0);
        if (text != MAP_FAILED) {
            keyboard->keymap.reset(xkb_keymap_new_from_buffer(keyboard->context, static_cast<const char*>(text), size,
                                                              XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
            munmap(text, size);
        }
    }
    close(descriptor);
}

void onEnter(void* /*data*/, wl_keyboard* /*keyboard*/, std::uint32_t /*serial*/, wl_surface* /*surface*/,
             wl_array* /*keys*/)
{
}

void onLeave(void* /*data*/, wl_keyboard* /*keyboard*/, std::uint32_t /*serial*/, wl_surface* /*surface*/) {}

void onKey(void* /*data*/, wl_keyboard* /*keyboard*/, std::uint32_t /*serial*/, std::uint32_t /*time*/,
           std::uint32_t /*key*/, std::uint32_t /*state*/)
{
}

void onModifiers(void* /*data*/, wl_keyboard* /*keyboard*/, std::uint32_t /*serial*/, std::uint32_t /*depressed*/,
                 std::uint32_t /*latched*/, std::uint32_t /*locked*/, std::uint32_t /*group*/)
{
}

void onRepeatInfo(void* /*data*/, wl_keyboard* /*keyboard*/, std::int32_t /*rate*/, std::int32_t /*delay*/) {}

KeymapPointer keymapOfWayland(xkb_context* context)
{
    wl_display* display = wl_display_connect(nullptr);
    if (display == nullptr) {
        return nullptr;
    }
    WaylandKeyboard keyboard{.context = context, .seat = nullptr, .keymap = nullptr};
    static const wl_registry_listener registryListener{.global = onGlobal, .global_remove = onGlobalRemove};
    static const wl_keyboard_listener keyboardListener{.keymap = onKeymap, .enter = onEnter, .leave = onLeave,
                                                       .key = onKey, .modifiers = onModifiers,
                                                       .repeat_info = onRepeatInfo};
    wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registryListener, &keyboard);
    wl_display_roundtrip(display);
    if (keyboard.seat != nullptr) {
        wl_keyboard* waylandKeyboard = wl_seat_get_keyboard(keyboard.seat);
        wl_keyboard_add_listener(waylandKeyboard, &keyboardListener, &keyboard);
        wl_display_roundtrip(display);
        wl_keyboard_destroy(waylandKeyboard);
        wl_seat_destroy(keyboard.seat);
    }
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return std::move(keyboard.keymap);
}

}  // namespace

struct KeyboardLayouts::Keymap
{
    ContextPointer context;
    KeymapPointer keymap;
    std::vector<std::optional<bool>> nonLatin;  // per layout, worked out when first needed
};

bool isOtherScriptLetter(char32_t character)
{
    if (character == 0 || character > QChar::LastValidCodePoint ||
        !(QChar::isLetter(character) || QChar::isMark(character))) {
        return false;
    }
    const QChar::Script script = QChar::script(character);
    return script != QChar::Script_Latin && script != QChar::Script_Common && script != QChar::Script_Inherited;
}

char32_t characterOfKeySymbol(std::uint32_t keySymbol)
{
    return xkb_keysym_to_utf32(keySymbol);
}

KeyboardLayouts::KeyboardLayouts(Source source)
    : m_source(source)
{
}

KeyboardLayouts::KeyboardLayouts(std::string layouts, std::string variants)
    : m_source(Source::Defaults)
    , m_layouts(std::move(layouts))
    , m_variants(std::move(variants))
{
}

KeyboardLayouts::~KeyboardLayouts() = default;

KeyboardLayouts& KeyboardLayouts::system()
{
    static KeyboardLayouts layouts([] {
        const QString platform = QGuiApplication::platformName();
        if (platform == QLatin1String("xcb")) {
            return Source::X11;
        }
        if (platform.startsWith(QLatin1String("wayland"))) {
            return Source::Wayland;
        }
        return Source::Defaults;
    }());
    return layouts;
}

bool KeyboardLayouts::load()
{
    auto loaded = std::make_unique<Keymap>();
    loaded->context.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
    if (!loaded->context) {
        return false;
    }
    xkb_context* context = loaded->context.get();
    switch (m_source) {
    case Source::X11:
        // Under XWayland the X server takes its keyboard from the Wayland desktop, which therefore has the real one
        if (const char* wayland = std::getenv("WAYLAND_DISPLAY"); wayland != nullptr && *wayland != '\0') {  // NOLINT(concurrency-mt-unsafe)
            loaded->keymap = keymapOfWayland(context);
        }
        if (!loaded->keymap) {
            loaded->keymap = keymapOfX11(context);
        }
        break;
    case Source::Wayland: loaded->keymap = keymapOfWayland(context); break;
    case Source::Defaults:
        loaded->keymap = m_layouts.empty()
                             ? KeymapPointer(xkb_keymap_new_from_names(context, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS))
                             : keymapFromNames(context, {.rules = {}, .model = {}, .layouts = m_layouts,
                                                         .variants = m_variants, .options = {}});
        break;
    }
    if (!loaded->keymap) {
        return false;
    }
    loaded->nonLatin.resize(xkb_keymap_num_layouts(loaded->keymap.get()));
    m_keymap = std::move(loaded);
    return true;
}

// A layout is not Latin when more of its 26 letter keys type letters of another script than Latin letters
bool KeyboardLayouts::isNonLatinLayout(std::uint32_t layout)
{
    std::optional<bool>& known = m_keymap->nonLatin.at(layout);
    if (!known) {
        int other = 0;
        int latin = 0;
        for (std::uint32_t scanCode = 0; scanCode <= lastLetterScanCode; ++scanCode) {
            if (!latinLetterAtScanCode(scanCode)) {
                continue;
            }
            const xkb_keysym_t* symbols = nullptr;
            if (xkb_keymap_key_get_syms_by_level(m_keymap->keymap.get(), scanCode + xkbOffset, layout, 0, &symbols) < 1) {
                continue;
            }
            const char32_t character = characterOfKeySymbol(symbols[0]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (isOtherScriptLetter(character)) {
                ++other;
            } else if (character <= QChar::LastValidCodePoint && QChar::isLetter(character)) {
                ++latin;
            }
        }
        known = other > latin;
    }
    return *known;
}

// Read the layouts again, at most once every 2 seconds, so a key no layout gives costs no new reading on every press
bool KeyboardLayouts::loadIfDue()
{
    constexpr std::chrono::seconds pause{2};
    const auto now = std::chrono::steady_clock::now();
    if (m_lastLoad && now - *m_lastLoad < pause) {
        return false;
    }
    m_lastLoad = now;
    return load();
}

std::optional<bool> KeyboardLayouts::lookUp(XkbKey key)
{
    if (!m_keymap) {
        return std::nullopt;
    }
    bool found = false;
    bool fromLatin = false;
    xkb_keymap* keymap = m_keymap->keymap.get();
    const std::uint32_t layouts = xkb_keymap_num_layouts_for_key(keymap, key.code);
    for (std::uint32_t layout = 0; layout < layouts; ++layout) {
        const std::uint32_t levels = xkb_keymap_num_levels_for_key(keymap, key.code, layout);
        for (std::uint32_t level = 0; level < levels; ++level) {
            const xkb_keysym_t* symbols = nullptr;
            const int count = xkb_keymap_key_get_syms_by_level(keymap, key.code, layout, level, &symbols);
            bool gives = false;
            for (int index = 0; index < count; ++index) {
                gives = gives || symbols[index] == key.symbol;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
            if (gives) {
                found = true;
                fromLatin = fromLatin || !isNonLatinLayout(layout);
                break;
            }
        }
    }
    if (!found) {
        return std::nullopt;
    }
    return !fromLatin;
}

bool KeyboardLayouts::isFromNonLatinLayout(XkbKey key)
{
    if (!m_keymap) {
        loadIfDue();
    }
    if (const std::optional<bool> nonLatin = lookUp(key)) {
        return *nonLatin;
    }
    // No layout gives it: one may have been added since
    if (loadIfDue()) {
        return lookUp(key).value_or(false);
    }
    return false;
}

}  // namespace chrono_calc
