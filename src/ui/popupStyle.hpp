#pragma once
// === C++ Libraries ===
#include <algorithm>  // Applies std::min/std::max for corner clamping.
#include <cmath>      // Computes arc points for the rounded corners.
#include <cstddef>    // Supplies std::size_t used in the point count.
// === SFML Libraries ===
#include <SFML/Graphics/Rect.hpp>      // Defines sf::FloatRect used for popup bounds.
#include <SFML/Graphics/RenderTarget.hpp>  // Draws the popup background.
#include <SFML/Graphics/Shape.hpp>     // Base class for the custom rounded shape.
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include "helper/colorHelper.hpp"       // Palette helpers used when drawing the frame.
#include "story/textStyles.hpp"         // Supplies the panel colors referenced below.

class RoundedRectangleShape : public sf::Shape {
public:
    RoundedRectangleShape(sf::Vector2f size = {}, float radius = 0.f, std::size_t cornerPointCount = 8)
    : m_size(size)
    , m_radius(radius)
    , m_cornerPointCount(std::max<std::size_t>(2, cornerPointCount))
    {
        update();
    }

    void setSize(sf::Vector2f size) {
        m_size = size;
        update();
    }

    void setCornerRadius(float radius) {
        m_radius = radius;
        update();
    }

    void setCornerPointCount(std::size_t count) {
        m_cornerPointCount = std::max<std::size_t>(2, count);
        update();
    }

    [[nodiscard]] sf::Vector2f getSize() const {
        return m_size;
    }

    [[nodiscard]] float getCornerRadius() const {
        return m_radius;
    }

    [[nodiscard]] std::size_t getPointCount() const override {
        return m_cornerPointCount * 4;
    }

    [[nodiscard]] sf::Vector2f getPoint(std::size_t index) const override {
        float radius = std::min(m_radius, std::min(m_size.x, m_size.y) * 0.5f);
        std::size_t corner = index / m_cornerPointCount;
        std::size_t pointIndex = index % m_cornerPointCount;

        if (radius <= 0.f) {
            return {
                (corner % 2) * m_size.x,
                (corner / 2) * m_size.y
            };
        }

        constexpr float kPi = 3.14159265358979323846f;
        constexpr float kQuarter = kPi * 0.5f;
        float progress = static_cast<float>(pointIndex) / static_cast<float>(m_cornerPointCount - 1);
        float startAngle = 0.f;
        sf::Vector2f center;

        switch (corner) {
            case 0:
                center = { radius, radius };
                startAngle = kPi;
                break;
            case 1:
                center = { m_size.x - radius, radius };
                startAngle = 1.5f * kPi;
                break;
            case 2:
                center = { m_size.x - radius, m_size.y - radius };
                startAngle = 0.f;
                break;
            default:
                center = { radius, m_size.y - radius };
                startAngle = 0.5f * kPi;
                break;
        }

        float angle = startAngle + progress * kQuarter;
        return center + sf::Vector2f{ std::cos(angle), std::sin(angle) } * radius;
    }

private:
    sf::Vector2f m_size;
    float m_radius = 0.f;
    std::size_t m_cornerPointCount = 4;
};

namespace ui::popup {
    inline constexpr float kLineSpacingMultiplier = 14.f;
    inline void drawPopupFrame(
        sf::RenderTarget& target,
        const sf::FloatRect& bounds,
        float alphaFactor,
        float radius = 18.f,
        std::size_t cornerPointCount = 20
    ) {
        RoundedRectangleShape frame({ bounds.size.x, bounds.size.y }, radius, cornerPointCount);
        frame.setPosition({ bounds.position.x, bounds.position.y });
        frame.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, alphaFactor));
        frame.setOutlineThickness(2.f);
        frame.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::FrameGoldLight, alphaFactor));
        target.draw(frame);
    }
}
