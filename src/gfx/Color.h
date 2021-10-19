#pragma once

class Color
{
    private:
    char value;
    static char hue;

    public:
    #ifdef USE_ATARI_COLORS
        static void setHue(char) {};
    #else
        static void setHue(char _hue) {hue = _hue << 4;};
    #endif
    
    Color() : value(0) {};
    #ifdef USE_ATARI_COLORS
        Color(char _value) : value(_value) {};
    #else
        Color(char _value) : value(hue | min(54, (int)_value) * 15 / 54) {};
    #endif
    
    Color& operator+=(const Color rhs) {
        value = (value & 0xF0) | min(16, value & 0x0F + rhs.value & 0x0F);
        return *this;
    }
    
    Color& operator*=(const float rhs) {
        value = (value & 0xF0) | min(16, int((value & 0x0F) * rhs + 0.5));
        return *this;
    }
 
    friend Color operator+(Color lhs, const Color rhs) {
        lhs += rhs;
        return lhs;
    }
    
    friend Color operator+(Color lhs, const char value) {
        lhs += value;
        return lhs;
    }
    
    friend Color operator*(Color lhs, const float value) {
        lhs *= value;
        return lhs;
    }
    
    operator char() {
        return value;
    }
};


