#pragma once

#include <iostream>
#include <string>
#include <format>
#include <cstdint>

namespace Terminal
{
    class Modifier
    {
    public:
        explicit Modifier(int code) : code(code) { }

        friend std::ostream& operator<<(std::ostream& os, const Modifier& mod);
        friend std::string&  operator+ (std::string&   s, const Modifier& mod);

    private:
        int code;
    };

    class Command
    {
    public:
        explicit Command(const std::string& sequence) : sequence(sequence) { }

        friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
        friend std::string&  operator+ (std::string&   s, const Command& cmd);

    private:
        std::string sequence;
    };

    // Foreground Colors
    class Color : public Modifier
    {
    public:
        static const Color Default;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Yellow;
        static const Color Blue;
        static const Color Magenta;
        static const Color Cyan;
        static const Color LightGray;
        static const Color DarkGray;
        static const Color LightRed;
        static const Color LightGreen;
        static const Color LightYellow;
        static const Color LightBlue;
        static const Color LightMagenta;
        static const Color LightCyan;
        static const Color White;

        explicit Color(int code) : Modifier(code) { }
    };

    // Background Colors
    class BackgroundColor : public Modifier
    {
    public:
        static const BackgroundColor Default;
        static const BackgroundColor Black;
        static const BackgroundColor Red;
        static const BackgroundColor Green;
        static const BackgroundColor Yellow;
        static const BackgroundColor Blue;
        static const BackgroundColor Magenta;
        static const BackgroundColor Cyan;
        static const BackgroundColor LightGray;
        static const BackgroundColor DarkGray;
        static const BackgroundColor LightRed;
        static const BackgroundColor LightGreen;
        static const BackgroundColor LightYellow;
        static const BackgroundColor LightBlue;
        static const BackgroundColor LightMagenta;
        static const BackgroundColor LightCyan;
        static const BackgroundColor White;

        explicit BackgroundColor(int code) : Modifier(code) { }
    };

    // Text Styles
    class Style : public Modifier
    {
    public:
        static const Style Reset;
        static const Style Bold;
        static const Style Dim;
        static const Style Italic;
        static const Style Underline;
        static const Style Blink;
        static const Style Reverse;
        static const Style Hidden;
        static const Style Strikethrough;

        explicit Style(int code) : Modifier(code) { }
    };

    // Cursor manipulation commands
    namespace Cursor
    {
        // Variables (modifiers) for cursor control
        extern const Command SavePosition;
        extern const Command RestorePosition;
        extern const Command ClearScreen;
        extern const Command ClearLine;

        // Functions for cursor movement with parameters
        std::string moveUp   (uint16_t n = 1);
        std::string moveDown (uint16_t n = 1);
        std::string moveRight(uint16_t n = 1);
        std::string moveLeft (uint16_t n = 1);
        std::string moveTo   (uint16_t row, uint16_t col);
    }

}
