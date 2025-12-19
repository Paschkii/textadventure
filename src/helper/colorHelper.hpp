#pragma once
// === C++ Libraries ===
#include <string>      // Stores hex codes and descriptive strings used in parsers.
#include <stdexcept>   // Throws when encountering invalid hex color strings.
#include <cstdio>      // Uses std::sscanf to decode hexadecimal color components.
#include <cstdint>     // Provides precise integer types for constructing sf::Color.
#include <algorithm>   // Supplies clamp/min/max helpers for color adjustments.
// === SFML Libraries ===
#include <SFML/Graphics/Color.hpp>  // Defines sf::Color used throughout the helper palette.

namespace ColorHelper {
    // ---------------
    // HEX » sf::Color
    // ---------------
    // Parses a hex string (#RRGGBB or #RRGGBBAA) into an sf::Color.
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
    
    // Replaces the alpha channel on the provided color.
    inline sf::Color alpha(sf::Color c, std::uint8_t a) {
        c.a = a;
        return c;
    }

    // Scales the color's alpha by the supplied factor.
    inline sf::Color applyAlphaFactor(sf::Color color, float factor) {
        color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * factor);
        return color;
    }

    // Decreases all RGB channels to darken the color.
    inline sf::Color darken(sf::Color c, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);
        c.r = static_cast<std::uint8_t>(c.r * (1.f - amount));
        c.g = static_cast<std::uint8_t>(c.g * (1.f - amount));
        c.b = static_cast<std::uint8_t>(c.b * (1.f - amount));
        return c;
    }

    // Moves RGB channels toward white to lighten the color.
    inline sf::Color lighten(sf::Color c, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);
        c.r = static_cast<std::uint8_t>(c.r + (255 - c.r) * amount);
        c.g = static_cast<std::uint8_t>(c.g + (255 - c.g) * amount);
        c.b = static_cast<std::uint8_t>(c.b + (255 - c.b) * amount);
        return c;
    }

    // Blends the color toward its grayscale equivalent.
    inline sf::Color desaturate(sf::Color c, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);
        std::uint8_t gray = (c.r + c.g + c.b) / 3;
        c.r = static_cast<std::uint8_t>(c.r + (gray - c.r) * amount);
        c.g = static_cast<std::uint8_t>(c.g + (gray - c.g) * amount);
        c.b = static_cast<std::uint8_t>(c.b + (gray - c.b) * amount);
        return c;
    }

    // Linearly interpolates between the base color and the tint color.
    inline sf::Color tint(sf::Color c, sf::Color t, float amount) {
        amount = std::clamp(amount, 0.f, 1.f);

        c.r = static_cast<std::uint8_t>(c.r + (t.r - c.r) * amount);
        c.g = static_cast<std::uint8_t>(c.g + (t.g - c.g) * amount);
        c.b = static_cast<std::uint8_t>(c.b + (t.b - c.b) * amount);
        return c;
    }


    // --------------------
    // PRE-DEFINED PALETTES
    // --------------------
    namespace Palette {
        inline const sf::Color Purple = sf::Color(0xC1, 0xA1, 0xFF);           // Primary accent used on highlighted UI edges.
        inline const sf::Color BlueDark = sf::Color(0x1C, 0x25, 0x44);         // Deep background tone for windows.
        inline const sf::Color BlueNearBlack = sf::Color(10, 12, 20);          // Near-black used for the highest contrast backdrops.
        inline const sf::Color DialogBackdrop = sf::Color(12, 12, 18, 210);    // Semi-transparent black used behind dialogue.
        inline const sf::Color BlueAlsoNearBlack = sf::Color(0x1F, 0x2A, 0x3B);// Also Near Black Color
        inline const sf::Color NavyBlue = sf::Color(0x4F, 0x6A, 0x8A);         // Navy Blue Color
        inline const sf::Color Blue = sf::Color(0x2F, 0x3B, 0x68);             // Mid-tone blue used for panels and text outlines.
        inline const sf::Color BlueLight = sf::Color(0x7C, 0x95, 0xCC);        // Light blue shading for highlights.
        inline const sf::Color MetalDark = sf::Color(0x2F, 0x32, 0x38);        // Steel tone for frame borders.
        inline const sf::Color Green = sf::Color(0x80, 0xB6, 0x63);            // Success/positive signal color.
        inline const sf::Color Health = sf::Color(0xE4, 0x44, 0x44);           // Health indicator for HP bars.
        inline const sf::Color Orange = sf::Color(0xFF, 0xC7, 0x9F);           // Warm UI accent for buttons.
        inline const sf::Color SoftOrange = sf::Color(0xFF, 0xCE, 0xA2);       // Subtle orange for secondary accents.
        inline const sf::Color Pink = sf::Color(0xFF, 0xAD, 0xC6);             // Decorative pink used in overlays.
        inline const sf::Color DarkBrown = sf::Color(0x5A, 0x3E, 0x36);        // Deep brown for wood/stony sprites.
        inline const sf::Color LightBrown = sf::Color(0x9C, 0x6B, 0x4E);       // Lighter brown for frames.
        inline const sf::Color SoftYellow = sf::Color(0xF2, 0xD3, 0x64);       // Muted yellow used for info callouts.
        inline const sf::Color SkyBlue = sf::Color(0x7C, 0xC6, 0xF9);          // Sky tone used in dragon/art accents.
        inline const sf::Color FrameGoldLight = sf::Color(0xF7, 0xE5, 0x7A);   // Bright gold for UI frame highlights.
        inline const sf::Color FrameGoldDark = sf::Color(0xC6, 0x9A, 0x26);    // Dark gold for frame shadows.
        inline const sf::Color White20 = sf::Color(255, 255, 255, 20);         // Light overlay tint at 8% alpha.
        inline const sf::Color White40 = sf::Color(255, 255, 255, 40);         // Semi-transparent white used for glows.
        inline const sf::Color White80 = sf::Color(255, 255, 255, 80);         // Brighter white for strong highlights.
        inline const sf::Color DarkPurple = sf::Color(80, 45, 115);            // Specialty purple for story-critical screens.
        inline const sf::Color Overlay = sf::Color(0, 0, 0, 120);              // Dark overlay color used on modal masks.
        inline const sf::Color Mint = sf::Color(0xA8, 0xFF, 0xE0);             // Mint tone for positive text glows.
        inline const sf::Color Rose = sf::Color(0xF9, 0xA8, 0xC5);             // Rosy tint used for story highlights.
        inline const sf::Color Amber = sf::Color(0xFF, 0xC4, 0x63);            // Amber hue used for interactive accents.
        inline const sf::Color IceBlue = sf::Color(0xB4, 0xE1, 0xFF);          // Cold blue for ice/dragon references.
        inline const sf::Color Normal = sf::Color(0xFF, 0xFF, 0xFF);           // Standard white for general text.
        inline const sf::Color Subtitle = sf::Color(0xD0, 0xD0, 0xD0);         // Lighter grey used for subtitle text.
        inline const sf::Color Dim = sf::Color(0xAA, 0xAA, 0xAA);              // Dim grey for disabled UI elements.
        inline const sf::Color NpcHero = sf::Color(0x7C, 0xCC, 0xF9);          // Hero NPC speech color.
        inline const sf::Color SoftRed = sf::Color(0xFF, 0x66, 0x66);          // Soft Red color.
        inline const sf::Color NpcMage = sf::Color(0xB0, 0x7C, 0xFF);          // Mage speech color.
        inline const sf::Color NpcRogue = sf::Color(0xB4, 0xFF, 0xA1);         // Rogue speech color.
        inline const sf::Color Shadow40 = sf::Color(0, 0, 0, 40);              // Light shadow gradient at 16% alpha.
        inline const sf::Color Shadow80 = sf::Color(0, 0, 0, 80);              // Medium shadow for hover states.
        inline const sf::Color Shadow120 = sf::Color(0, 0, 0, 120);            // Heavier shadow for dark panels.
        inline const sf::Color Shadow180 = sf::Color(0, 0, 0, 180);            // Nearly opaque shadow for modals.
        // Additional palette entries used across UI
        inline const sf::Color Sepia = sf::Color(180, 160, 130);               // Sepia tone for aged UI elements.
        inline const sf::Color SepiaDark = sf::Color(90, 72, 54);              // Darker sepia for outlines.
        inline const sf::Color TitleAccent = sf::Color(255, 186, 59);          // Accent gold for the title screen.
        inline const sf::Color TitleBack = sf::Color(92, 64, 35);              // Brown background used during the intro title.
        inline const sf::Color IconGray = sf::Color(160, 160, 160);            // Neutral gray for small icons.
        inline const sf::Color PromptGray = sf::Color(200, 200, 200);          // Lighter gray used for prompts.
        // Speaker-specific colors (used by TextStyles)
        inline const sf::Color PurpleBlue = sf::Color(180, 180, 255);          // Extra light color for supportive NPCs.
        inline const sf::Color SpeakerBrown = sf::Color(139, 69, 19);          // Default brown tone for speaker names.
        inline const sf::Color PlayerColor = sf::Color::Cyan;                  // Player dialogue color.
        inline const sf::Color FireDragon = sf::Color::Red;                    // Fire dragon speaker color.
        inline const sf::Color WaterDragon = sf::Color::Blue;                  // Water dragon speaker color.
        inline const sf::Color AirDragon = sf::Color::Green;                   // Air dragon speaker color.
        inline const sf::Color EarthDragon = SpeakerBrown;                     // Earth dragon reuses the speaker brown tone.
    }
}
