#pragma once
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <cstdint>
#include <algorithm>

namespace ColorHelper {
    // ----------------------------------------------------------
    //  HEX » sf::Color
    // ----------------------------------------------------------
    inline sf::Color hex(const std::string& code)
    {
        unsigned int r = 0, g = 0, b = 0, a = 255;
        
        if (code.size() == 7) // #RRGGBB
        {
            std::sscanf(code.c_str(), "#%02x%02x%02x", &r, &g, &b);
        }
        else if (code.size() == 9) // #RRGGBBAA
        {
            std::sscanf(code.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);
        }
        else
        {
            throw std::runtime_error("Invalid hex color: " + code);
        }
        
        return sf::Color(
            static_cast<std::uint8_t>(r),
            static_cast<std::uint8_t>(g),
            static_cast<std::uint8_t>(b),
            static_cast<std::uint8_t>(a)
        );
    }
    
    inline sf::Color alpha(sf::Color c, std::uint8_t a) {
        c.a = a;
        return c;
    }

    inline sf::Color applyAlphaFactor(sf::Color color, float factor) {
        color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * factor);
        return color;
    }

    inline sf::Color darken(sf::Color c, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);
        c.r = static_cast<std::uint8_t>(c.r * (1.f - amount));
        c.g = static_cast<std::uint8_t>(c.g * (1.f - amount));
        c.b = static_cast<std::uint8_t>(c.b * (1.f - amount));
        return c;
    }

    inline sf::Color lighten(sf::Color c, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);
        c.r = static_cast<std::uint8_t>(c.r + (255 - c.r) * amount);
        c.g = static_cast<std::uint8_t>(c.g + (255 - c.g) * amount);
        c.b = static_cast<std::uint8_t>(c.b + (255 - c.b) * amount);
        return c;
    }

    inline sf::Color desaturate(sf::Color c, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);
        std::uint8_t gray = (c.r + c.g + c.b) / 3;
        c.r = static_cast<std::uint8_t>(c.r + (gray - c.r) * amount);
        c.g = static_cast<std::uint8_t>(c.g + (gray - c.g) * amount);
        c.b = static_cast<std::uint8_t>(c.b + (gray - c.b) * amount);
        return c;
    }

    inline sf::Color tint(sf::Color c, sf::Color t, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);

        c.r = static_cast<std::uint8_t>(c.r + (t.r - c.r) * amount);
        c.g = static_cast<std::uint8_t>(c.g + (t.g - c.g) * amount);
        c.b = static_cast<std::uint8_t>(c.b + (t.b - c.b) * amount);
        return c;
    }


    // ----------------------------------------------------------
    //  VORDEFINIERTE PALETTEN
    // ----------------------------------------------------------
    namespace Palette {
        inline const sf::Color Purple = sf::Color(0xC1, 0xA1, 0xFF);
        inline const sf::Color BlueDark = sf::Color(0x1C, 0x25, 0x44);
        inline const sf::Color BlueNearBlack = sf::Color(10, 12, 20);
        inline const sf::Color Blue = sf::Color(0x2F, 0x3B, 0x68);
        inline const sf::Color BlueLight = sf::Color(0x7C, 0x95, 0xCC);
        inline const sf::Color MetalDark = sf::Color(0x2F, 0x32, 0x38);
        inline const sf::Color Green = sf::Color(0x80, 0xB6, 0x63);
        inline const sf::Color Orange = sf::Color(0xFF, 0xC7, 0x9F);
        inline const sf::Color SoftOrange = sf::Color(0xFF, 0xCE, 0xA2);
        inline const sf::Color Pink = sf::Color(0xFF, 0xAD, 0xC6);
        inline const sf::Color DarkBrown = sf::Color(0x5A, 0x3E, 0x36);
        inline const sf::Color LightBrown = sf::Color(0x9C, 0x6B, 0x4E);
        inline const sf::Color SoftYellow = sf::Color(0xF2, 0xD3, 0x64);
        inline const sf::Color SkyBlue = sf::Color(0x7C, 0xC6, 0xF9);
        inline const sf::Color FrameGoldLight = sf::Color(0xF7, 0xE5, 0x7A);
        inline const sf::Color FrameGoldDark = sf::Color(0xC6, 0x9A, 0x26);
        inline const sf::Color White20 = sf::Color(255, 255, 255, 20);
        inline const sf::Color White40 = sf::Color(255, 255, 255, 40);
        inline const sf::Color White80 = sf::Color(255, 255, 255, 80);
        inline const sf::Color DarkPurple = sf::Color(80, 45, 115);
        inline const sf::Color Overlay = sf::Color(0, 0, 0, 120);
        inline const sf::Color Mint = sf::Color(0xA8, 0xFF, 0xE0);
        inline const sf::Color Rose = sf::Color(0xF9, 0xA8, 0xC5);
        inline const sf::Color Amber = sf::Color(0xFF, 0xC4, 0x63);
        inline const sf::Color IceBlue = sf::Color(0xB4, 0xE1, 0xFF);
        inline const sf::Color Normal = sf::Color(0xFF, 0xFF, 0xFF);
        inline const sf::Color Subtitle = sf::Color(0xD0, 0xD0, 0xD0);
        inline const sf::Color Dim = sf::Color(0xAA, 0xAA, 0xAA);
        inline const sf::Color NpcHero = sf::Color(0x7C, 0xCC, 0xF9);
        inline const sf::Color NpcVillain = sf::Color(0xFF, 0x66, 0x66);
        inline const sf::Color NpcMage = sf::Color(0xB0, 0x7C, 0xFF);
        inline const sf::Color NpcRogue = sf::Color(0xB4, 0xFF, 0xA1);
        inline const sf::Color Shadow40 = sf::Color(0, 0, 0, 40);
        inline const sf::Color Shadow80 = sf::Color(0, 0, 0, 80);
        inline const sf::Color Shadow120 = sf::Color(0, 0, 0, 120);
        inline const sf::Color Shadow180 = sf::Color(0, 0, 0, 180);
        // Additional palette entries used across UI
        inline const sf::Color Sepia = sf::Color(180, 160, 130);
        inline const sf::Color SepiaDark = sf::Color(90, 72, 54);
        inline const sf::Color TitleAccent = sf::Color(255, 186, 59);
        inline const sf::Color TitleBack = sf::Color(92, 64, 35);
        inline const sf::Color IconGray = sf::Color(160, 160, 160);
        inline const sf::Color PromptGray = sf::Color(200, 200, 200);
        // Speaker-specific colors (used by TextStyles)
        inline const sf::Color PurpleBlue = sf::Color(180, 180, 255);
        inline const sf::Color SpeakerBrown = sf::Color(139, 69, 19);
        inline const sf::Color PlayerColor = sf::Color::Cyan;
        inline const sf::Color FireDragon = sf::Color::Red;
        inline const sf::Color WaterDragon = sf::Color::Blue;
        inline const sf::Color AirDragon = sf::Color::Green;
        inline const sf::Color EarthDragon = SpeakerBrown;
    }
}
