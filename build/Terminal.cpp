#include "Terminal.h"

namespace Terminal
{
    std::ostream& operator<<(std::ostream& os, const Modifier& mod)
    {
        return os << "\033[" << mod.code << "m";
    }

    std::string&  operator+ (std::string&   s, const Modifier& mod)
    {
        return s += "\033[" + std::to_string(mod.code) + "m";
    }

    std::ostream& operator<<(std::ostream& os, const Command& cmd)
    {
        return os << cmd.sequence;
    }
    
    std::string&  operator+ (std::string&   s, const Command& cmd)
    {
        return s += cmd.sequence;
    }

    // Definitions of static members
    // Foreground Colors
    const Color Color::Default     (39);
    const Color Color::Black       (30);
    const Color Color::Red         (31);
    const Color Color::Green       (32);
    const Color Color::Yellow      (33);
    const Color Color::Blue        (34);
    const Color Color::Magenta     (35);
    const Color Color::Cyan        (36);
    const Color Color::LightGray   (37);
    const Color Color::DarkGray    (90);
    const Color Color::LightRed    (91);
    const Color Color::LightGreen  (92);
    const Color Color::LightYellow (93);
    const Color Color::LightBlue   (94);
    const Color Color::LightMagenta(95);
    const Color Color::LightCyan   (96);
    const Color Color::White       (97);

    // Background Colors
    const BackgroundColor BackgroundColor::Default     ( 49);
    const BackgroundColor BackgroundColor::Black       ( 40);
    const BackgroundColor BackgroundColor::Red         ( 41);
    const BackgroundColor BackgroundColor::Green       ( 42);
    const BackgroundColor BackgroundColor::Yellow      ( 43);
    const BackgroundColor BackgroundColor::Blue        ( 44);
    const BackgroundColor BackgroundColor::Magenta     ( 45);
    const BackgroundColor BackgroundColor::Cyan        ( 46);
    const BackgroundColor BackgroundColor::LightGray   ( 47);
    const BackgroundColor BackgroundColor::DarkGray    (100);
    const BackgroundColor BackgroundColor::LightRed    (101);
    const BackgroundColor BackgroundColor::LightGreen  (102);
    const BackgroundColor BackgroundColor::LightYellow (103);
    const BackgroundColor BackgroundColor::LightBlue   (104);
    const BackgroundColor BackgroundColor::LightMagenta(105);
    const BackgroundColor BackgroundColor::LightCyan   (106);
    const BackgroundColor BackgroundColor::White       (107);

    // Text Styles
    const Style Style::Reset        (0);
    const Style Style::Bold         (1);
    const Style Style::Dim          (2);
    const Style Style::Italic       (3);
    const Style Style::Underline    (4);
    const Style Style::Blink        (5);
    const Style Style::Reverse      (7);
    const Style Style::Hidden       (8);
    const Style Style::Strikethrough(9);

    // Cursor manipulation commands
    namespace Cursor
    {
        const Command SavePosition   ("\033[s");
        const Command RestorePosition("\033[u");
        const Command ClearScreen    ("\033[2J\033[H");
        const Command ClearLine      ("\033[2K\r");

        // Functions for cursor movement with parameters
        std::string moveUp(uint16_t n)
        {
            return "\033[" + std::to_string(n) + "A";
        }

        std::string moveDown(uint16_t n)
        {
            return "\033[" + std::to_string(n) + "B";
        }

        std::string moveRight(uint16_t n)
        {
            return "\033[" + std::to_string(n) + "C";
        }

        std::string moveLeft(uint16_t n)
        {
            return "\033[" + std::to_string(n) + "D";
        }

        std::string moveTo(uint16_t row, uint16_t col)
        {
            return "\033[" + std::to_string(row) + ";" + std::to_string(col) + "H";
        }
    }

}
