/*

More or less ANSI escape codes for POSIX targets

*/

/*
Questions for myself

- How often does stdout flush, and how does that change on a per system basis
    or with changes to TCSETATTR

- How do we offer support for wide char unicode (or UTF-16?)
    A: I am converting them, ez

- We are removing bounds checks on write, it seems unimportant
    A: True, programmer can figure that one out, no need to add overhead

- How do nested alternate buffers work? By pid?
    A: Fine, pipe isn't going to affect the terminal

- Seems to be some multiple construction, please verify

*/

#define CONSOLE_ANSI

#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include <string.h>
#include <cassert>
#include <termios.h>

#include "console.h"

/*
    console.h
*/
console::constructor console::cons;
bool console::ready = false;

/*
    Not exposed by a header yet
*/
static bool already_constructed = false;
bool activate_alternate_screen_by_default = true;
bool reset_text_mode_by_default = true;
bool is_cursor_visible = false;
bool is_alternate_screen = false;
// Probably won't use this
bool is_standard_color_mode = true;
// If true don't use terminal's default palette
bool is_ansi_256_color_mode = true;
color_t active_foreground_color_mode = 0;
color_t active_background_color_mode = 0;
termios termios_start;

inline void setAlternateBuffer(const bool &state);
inline void setLineBuffering(const bool &state);
inline void setDEC(const int &code, const bool &state);
inline void toggleAlternateBuffer();
inline void enableAlternateBuffer();
inline void disableAlternateBuffer();

template<typename... Modes>
inline void setModes(const Modes ...modes);

namespace DEC_CODES {
    enum Codes {
        CURSOR_VISIBILITY = 25,
        SAVE_RESTORE_SCREEN = 47,
        REPORT_MOUSE_PRESS_RELEASE = 1000,
        REPORT_MOUSE_MOVEMENT_BUTTON_PRESS = 1002,
        REPORT_MOUSE_MOVEMENT = 1003,
        REPORT_INPUT_FOCUS = 1004,
        MODERN_MOUSE_MOVEMENT = 1006,
        ALTERNATE_SCROLL = 1007
    };
};

console::constructor::constructor() {
    if (already_constructed) //issues with multiple construction
        return;

    if (activate_alternate_screen_by_default)
        enableAlternateBuffer();

    if (reset_text_mode_by_default)
        setModes(0); // Reset modes

    setDEC(DEC_CODES::CURSOR_VISIBILITY, false);

    tcgetattr(STDIN_FILENO, &termios_start);

    setLineBuffering(true);

    ready = true;
    already_constructed = true;
}

console::constructor::~constructor() {
    if (!already_constructed)
        return;

    ready = false;
    already_constructed = false;

    setLineBuffering(false);

    tcsetattr(STDIN_FILENO, TCSANOW, &termios_start);

    setDEC(DEC_CODES::CURSOR_VISIBILITY, true);

    if (reset_text_mode_by_default)
        setModes(0); // Probably want this as default as well..?

    disableAlternateBuffer(); // Probably want this?
}

/*
    Colors

    204 for intensity
    127 for normal

    For compatibility with my ncurses version and palettes
    but clearly we can do more

    !! The first 15 colors are "standard" except they use the normal palette
*/

const color_t ansi_256_colors[16] = {
    16, // 000000 or 000 for ansi
    18, // 000087 blue
    28, // 008700 green
    100, // 878700 yellow
    88, // 870000 red
    90, // 870087 magenta
    30, // 008787 cyan
    251, // c6c6c6 light gray
    244, // 808080 dark gray
    20, // 0000d7 blue
    40, // 00d700 green
    184, // d7d700 yellow
    160, // d70000 red
    164, // d700d7 magenta
    44, // 00d7d7 cyan
    231  // ffffff white
};

/*
    Remap standard colors

    It's flip red/blue, and fg so add 10 for bg
*/

const color_t ansi_colors[16] = {
    30,
    34,
    32,
    33,
    31,
    35,
    36,
    37,
    90,
    94,
    92,
    93,
    91,
    95,
    96,
    97
};

inline void toAnsi256ColorMode(const color_t &color, color_t &fg, color_t &bg) {
    fg = ansi_256_colors[color & 0b00001111];
    bg = ansi_256_colors[(color & 0b11110000) >> 4];
}

inline void toAnsiColorMode(const color_t &color, color_t &fg, color_t &bg, const bool &swap_rb = true) {
    fg = ansi_colors[color & 0b00001111];
    bg = ansi_colors[(color & 0b11110000) >> 4] + 10;
    /*
    const color_t fgIntensity = ((color & FINST) == FINST) * 60 + 30;
    const color_t bgIntensity = ((color & BINST) == BINST) * 60 + 40;
    const color_t mask = -1 * swap_rb;
    const color_t cmask = FRED | FBLUE;
    const color_t rmask = mask ^ cmask;

    fg = color & 0b00000111;
    bg = color >> 4 & 0b00000111;

    fg ^= (fg == FRED || fg == FBLUE) * cmask;
    bg ^= (bg == FRED || bg == FBLUE) * cmask;
    
    fg += fgIntensity;
    bg += bgIntensity;
    */
}

/*
U+uvwxyz
U+0000      U+007F      0yyyzzzz 	
U+0080      U+07FF      110xxxyy    10yyzzzz 	
U+0800      U+FFFF      1110wwww 	10xxxxyy 	10yyzzzz 	
U+010000    U+10FFFF    11110uvv 	10vvwwww 	10xxxxyy 	10yyzzzz
*/

template<typename T, typename R = int>
inline R getUTF8char(const T &unicode, int &output_length) {
    const int size = sizeof(T);
    static_assert(size >= 2, "Input size should be at least 2");
    static_assert(sizeof(R) >= 4, "Output size should be at least 4");

    int bl = 0x2591;
    if (unicode < 0x00000080) {
        output_length = 1;
        return R(unicode & 0x7F);
    } else
    if (unicode < 0x00000800) {
        output_length = 2;
        R ret =(unicode & 0x0000003F);
        ret |= 0x80;
        ret <<= 8;
        ret |= (unicode & 0x00000FC0) >> 6;
        ret |= 0xC0;
        return ret;
    } else
    if (unicode < 0x00010000) {
        output_length = 3;
        R ret =(unicode & 0x0000003F);
        ret |= 0x80;
        ret <<= 8;
        ret |= (unicode & 0x00000FC0) >> 6;
        ret |= 0x80;
        ret <<= 8;
        ret |= (unicode & 0x000F3000) >> 12;
        ret |= 0xE0;
        return ret;
    } else
    if (unicode < 0x00011000) {
        output_length = 4;
        R ret =(unicode & 0x0000003F);
        ret |= 0x80;
        ret <<= 8;
        ret |= (unicode & 0x00000FC0) >> 6;
        ret |= 0x80;
        ret <<= 8;
        ret |= (unicode & 0x000F3000) >> 12;
        ret |= 0x80;
        ret <<= 8;
        ret |= (unicode & 0x00FC0000) >> 18;
        ret |= 0xF0;
        return ret;
    } else {
        output_length = 1;
        return R(unicode & 0x7F);
    }

    /*
    R utf = (unicode & 0x0003F);
    utf |= 0x80;
    utf <<= 8;
    utf |=  (unicode & 0x00FC0) >> 6;
    utf |= 0x80;
    utf <<= 8;
    utf |=  (unicode & 0xF3000) >> 12;
    utf |= 0xC0;

    output_length = 3;

    return utf;
    */
}

template<typename T, typename R = wchar_t>
inline R getUnicodeChar(const T &utf8_char, int &input_consumed, bool &invalid_char) {
    const int size = sizeof(T);
    const unsigned char *buffer = (const unsigned char*)&utf8_char;
    
    invalid_char = false;
    input_consumed = 1;

    if ((buffer[0] & 0x80) == 0) {
        input_consumed = 1;
        return R(buffer[0]);
    } else
    if ((buffer[0] & 0xE0) == 0xC0) {
        input_consumed = 2;
        if ((buffer[1] & 0xC0) != 0x80)
            return R(0);
        R ret = buffer[0] & 0x1F;
        ret <<= 6;
        ret |= buffer[1] & 0x3F;
        return ret;
    } else
    if ((buffer[0] & 0xF0) == 0xE0) {
        input_consumed = 3;
        if ((buffer[1] & 0xC0) != 0x80 || (buffer[2] & 0xC0) != 0x80)
            return R(0);
        R ret = buffer[0] & 0x0F;
        ret <<= 6;
        ret |= buffer[1] & 0x3F;
        ret <<= 6;
        ret |= buffer[2] & 0x3F;
        return ret;
    } else
    if ((buffer[0] & 0xF8) == 0xF0) {
        input_consumed = 4;
        if ((buffer[1] & 0xC0) != 0x80 || (buffer[2] & 0xC0) != 0x80 || (buffer[3] & 0xC0) != 0x80)
            return R(0);
        R ret = buffer[0] & 0x07;
        ret <<= 6;
        ret |= buffer[1] & 0x3F;
        ret <<= 6;
        ret |= buffer[2] & 0x3F;
        ret <<= 6;
        ret |= buffer[3] & 0x3F;
        return ret;
    } else {
        input_consumed = 0;
        invalid_char = true;
        return R(0);
    }

}

/*
    source: 8-bit utf-8 string to convert
    source_length: size of source in bytes
    dest: destination 16-bit unicode buffer
    dest_element_count: size of destination buffer in element count
    returns: number of encoded elements
*/
inline int toUnicodeStr(const char *source, const int &source_length, wchar_t *dest, const int &dest_element_count) {
    int output_index = 0, source_index = 0, input_consumed, c;
    bool conv_error;
    const int buflen = 4;
    char buf[buflen];

    while (1) {
        if (output_index + 1 > dest_element_count)
            break;
        if (source_index + 1 > source_length)
            break;

        *((int*)&buf[0]) = 0;
        for (int i = 0; i < buflen && source_index + i < source_length; i++)
            buf[i] = source[source_index + i];

        c = getUnicodeChar(buf, input_consumed, conv_error);
        
        if (input_consumed < 1)
            input_consumed = 1;
        
        dest[output_index] = (decltype(*dest))c;

        source_index += input_consumed;
        output_index++;
    }

    return output_index;
}

/*
    source: 16-bit unicode string to convert
    source_element_count: size of source buffer in element count
    dest: destination 8-bit utf-8 buffer
    dest_length: size of dest in bytes
    returns: number of encoded bytes
*/
inline int toUTF8Str(const wchar_t *source, const int &source_element_count, char *dest, const int &dest_length) {
    int output_index = 0, char_size, c;
    char *pc;

    for (int i = 0; i < source_element_count; i++) {
        c = getUTF8char(source[i], char_size);

        if (output_index + char_size + 1 > dest_length)
            break;

        pc = (char*)&c;
    
        for (int j = 0; j < char_size; j++)
            dest[output_index+j] = pc[j];

        output_index += char_size;
    }

    return output_index;
}

inline winsize getWinSize() {
    winsize ws;
    int error = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (error == -1) {
        //Probably not a standard terminal we are outputting to?
        ws.ws_col = 80;
        ws.ws_row = 25;
    }
    return ws;
}

inline void writeChar(const char &ch) {
    int e = write(STDOUT_FILENO, &ch, 1);
}

inline void writeWChar(const wchar_t &wch) {
    int output_length;
    int c = getUTF8char(wch, output_length);
    int e = write(STDOUT_FILENO, &c, output_length);
}

inline void writeCharString(const char *str) {
    int e = write(STDOUT_FILENO, str, strlen(str));
}

inline void writeWCharString(const wchar_t *wstr) {
    const int wstrlen = std::wcslen(wstr);
    const int buflen = wstrlen * 3;
    char buf[buflen];
    int count = toUTF8Str(wstr, wstrlen, buf, buflen);
    int e = write(STDOUT_FILENO, buf, count);
}

inline void writeString(const std::string &str) {
    int e = write(STDOUT_FILENO, str.c_str(), str.size());
}

inline void writeWString(const std::wstring &str) {
    const int buflen = str.size() * 4;
    char buf[buflen];
    int count = toUTF8Str(str.data(), str.size(), buf, buflen);
    int e = write(STDOUT_FILENO, buf, count);
}

inline void setDEC(const int &code, const bool &state) {
    writeString("\x1b[?" + std::to_string(code) + (state ? "h" : "l"));
}

inline void setLineBuffering(const bool &state) {
    termios c;
    memcpy(&c, &termios_start, sizeof(termios));
    c.c_lflag &= (~ICANON | ~ECHO);
    if (state)
        c.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &c);
}

inline void setAlternateBuffer(const bool &state) {
    if (is_alternate_screen == state)
        return;

    setDEC(1049, state);
    is_alternate_screen = state;
}

inline void toggleAlternateBuffer() {
    setAlternateBuffer(!is_alternate_screen);
}

inline void enableAlternateBuffer() {
    setAlternateBuffer(true);
}

inline void disableAlternateBuffer() {
    setAlternateBuffer(false);
}

int console::getImage() {
    return IMAGE_POSIX;
}

void console::sleep(int millis) {
    usleep(millis * 1000);
}

int console::getConsoleWidth() {
    winsize ws = getWinSize();
    return int(ws.ws_col);
}

int console::getConsoleHeight() {
    winsize ws = getWinSize();
    return int(ws.ws_row);
}

void console::clear() {
    writeCharString("\x1b[2J");
}

void console::setCursorLeft(int x) {
    writeString("\x1b[" + std::to_string(x+1) + "n");
}

void console::setCursorTop(int y) {
    //we don't know x
    writeString("\x1b[" + std::to_string(y+1) + ";H");
}

void console::setCursorPosition(int x, int y) {
    writeString("\x1b[" + std::to_string(y+1) + ";" + std::to_string(x+1) + "H");
}

int readInput() {
    /*
        This is not a good plan
        Use poll and O_NONBLOCK
        Do not expect single byte input    
    */

    int error;

    const int buflen = 1;
    char buf[buflen];

    error = read(STDIN_FILENO, buf, buflen);

    assert(error != -1 && "Not sure how common read failure is");

    return int(buf[0]);
}

int console::readKey() {
    /*
        Applies to readKeyAsync as well
    
        Originally wanted modifiers to be flags
        but now I should have discrimation against
        escape codes being passed to the program

        Except I use the readKey to "update everything"
        sometimes, because of the events

        curses has a KEY_RESIZE and KEY_MOUSE for this
        purpose

        In that case, we should process escape codes
    */ 

    return readInput();
}

int console::readKeyAsync() {
    /*
        Simply do not block
        This is simple with FIONREAD
    */

    int count, error;

    error = ioctl(STDIN_FILENO, FIONREAD, &count);

    assert(error != -1 && "Not sure how common ioctl returns errors");

    if (count < 1)
        return 0;

    return readInput();
}

inline void setForegroundMode(const color_t &fg_mode) {
    writeString("\x1b[1;" + std::to_string(fg_mode) + "m");
}

inline void setBackgroundMode(const color_t &bg_mode) {
    writeString("\x1b[1;" + std::to_string(bg_mode) + "m");
}

inline void setMode(const color_t &fg_mode, const color_t &bg_mode) {
    writeString("\x1b[2;" + std::to_string(fg_mode) + ";" + std::to_string(bg_mode) + "m");
}

template<typename ...Modes>
inline void setModes(const Modes... modes) {
    std::string p = "\x1b[";

    ((p += std::to_string(modes), p += ";"), ...);
    if (sizeof...(modes) > 0 && p.back() == ';')
        p.pop_back();

    p += "m";
    writeString(p);
}

/*
    We have to do this to compose a buffer, for now
*/
template<typename ...Modes>
inline std::string getModesStr(const Modes... modes) {
    std::string p = "\x1b[";

    ((p += std::to_string(modes), p += ";"), ...);
    if (sizeof...(modes) > 0 && p.back() == ';')
        p.pop_back();
    
    p += "m";
    return p;
}

namespace CB_OPTS {
    enum Types {
        NONE=0,
        FG=1,
        BG=2,
        BOTH=3
    };
};

inline std::string getConsoleColorRGBStr(const color_t &r, const color_t &g, const color_t &b, const bool &bg = false) {
    return getModesStr(bg * 10 + 38, 2, r, g, b);
}

inline std::string getConsoleColorAnsi256Str(const color_t &color, const CB_OPTS::Types &opts = CB_OPTS::BOTH) {
    color_t fg_mode, bg_mode;
    toAnsi256ColorMode(color, fg_mode, bg_mode);
    switch (opts) {
        case CB_OPTS::FG: return getModesStr(38, 5, fg_mode);
        case CB_OPTS::BG: return getModesStr(48, 5, bg_mode);
        case CB_OPTS::BOTH: return getModesStr(38, 5, fg_mode, 48, 5, bg_mode);
        default: return "";
    }
}

inline std::string getConsoleColorStr(const color_t &color, const CB_OPTS::Types &opts = CB_OPTS::BOTH) {
    if (is_ansi_256_color_mode)
        return getConsoleColorAnsi256Str(color);
    color_t fg_mode, bg_mode;
    toAnsiColorMode(color, fg_mode, bg_mode);
    switch (opts) {
        case CB_OPTS::BG: return getModesStr(2, fg_mode);
        case CB_OPTS::FG: return getModesStr(1, bg_mode);
        case CB_OPTS::BOTH: return getModesStr(2, fg_mode, 1, bg_mode);
        default: return "";
    }
}

/*
    Doesn't check if mode is set already, since this is big data
    Handle going back to normal characters somehow
*/
inline void setConsoleColorRGB(const color_t &r, const color_t &g, const color_t &b, bool bg=false) {
    setModes(bg * 10 + 38, 2, r, g, b);
}

inline void setConsoleColorAnsi256(const color_t &color) {
    color_t fg_mode, bg_mode;
    toAnsi256ColorMode(color, fg_mode, bg_mode);
    if (fg_mode != active_foreground_color_mode && bg_mode != active_background_color_mode)
        setModes(38, 5, fg_mode, 48, 5, bg_mode);
    else
    if (fg_mode != active_foreground_color_mode)
        setModes(38, 5, fg_mode);
    else
    if (bg_mode != active_background_color_mode)
        setModes(48, 5, bg_mode);
}

void console::setConsoleColor(color_t color) {
    if (is_ansi_256_color_mode)
        return setConsoleColorAnsi256(color);
    color_t fg_mode, bg_mode;
    toAnsiColorMode(color, fg_mode, bg_mode);
    if (fg_mode != active_foreground_color_mode && bg_mode != active_background_color_mode)
        setModes(fg_mode, bg_mode);        
    else 
    if (fg_mode != active_foreground_color_mode)
        setForegroundMode(fg_mode);
    else 
    if (bg_mode != active_background_color_mode)
        setBackgroundMode(bg_mode);

    active_foreground_color_mode = fg_mode;
    active_background_color_mode = bg_mode;
}

void console::write(int x, int y, std::string &str) {
    setCursorPosition(x, y);
    writeString(str);
}

void console::write(int x, int y, const char *str) {
    setCursorPosition(x, y);
    writeCharString(str);
}

void console::write(int x, int y, const char *str, color_t c) {
    setCursorPosition(x, y);
    setConsoleColor(c);
    writeCharString(str);
}

void console::write(int x, int y, std::string &str, color_t c) {
    setCursorPosition(x, y);
    setConsoleColor(c);
    writeString(str);
}

void console::write(char *fb, color_t *cb, int length) {
    std::string buf;
    color_t fg_mode, bg_mode, fg, bg;

    // Should be the right color conversion eventually
    toAnsiColorMode(*cb, fg_mode, bg_mode);
    for (int i = 0; i < length; i++) {
        const char ch = fb[i];
        const color_t color = cb[i];
        
        toAnsiColorMode(color, fg, bg);

        CB_OPTS::Types mode = CB_OPTS::Types((fg_mode == fg ? 1 : 0) | (bg_mode == bg ? 2 : 0));

        buf += getConsoleColorStr(color, mode);
        buf += ch;

        fg_mode = fg, bg_mode = bg;
        
        //setConsoleColor(cb[i]);
        //writeChar(fb[i]);
    }

    writeString(buf);
}

void console::write(wchar_t *fb, color_t *cb, int length) {
    std::string buf;
    color_t fg_mode, bg_mode, fg, bg;
    char utf8buf[4];

    toAnsiColorMode(*cb, fg_mode, bg_mode);
    for (int i = 0; i < length; i++) {
        const wchar_t ch = fb[i];
        const color_t color = cb[i];
        
        toAnsiColorMode(color, fg, bg);

        CB_OPTS::Types mode = CB_OPTS::Types((fg_mode == fg ? 1 : 0) | (bg_mode == bg ? 2 : 0));

        int len = toUTF8Str(&ch, 1, utf8buf, 4);

        buf += getConsoleColorStr(color, mode);
        buf += std::string(buf, len);

        fg_mode = fg, bg_mode = bg;

        //setConsoleColor(cb[i]);
        //writeWChar(fb[i]);
    }

    writeString(buf);
}

void console::write(int x, int y, wchar_t character) {
    setCursorPosition(x, y);
    writeWChar(character);
}

void console::write(int x, int y, wchar_t character, color_t color) {
    setCursorPosition(x, y);
    setConsoleColor(color);
    writeWChar(character);
}

void console::write(int x, int y, char character) {
    setCursorPosition(x, y);
    writeChar(character);
}

void console::write(int x, int y, char character, color_t color) {
    setCursorPosition(x, y);
    setConsoleColor(color);
    writeChar(character);
}