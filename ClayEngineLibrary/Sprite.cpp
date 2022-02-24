#include "pch.h"
#include "Sprite.h"

using namespace ClayEngine;
using namespace ClayEngine::Graphics;
using namespace ClayEngine::SimpleMath;

#pragma region BaseSprite
BaseSprite::BaseSprite()
{
    m_spritebatch = Services::GetService<RenderSystem>()->GetSpriteBatch();
}

BaseSprite::~BaseSprite()
{
    m_spritebatch = nullptr;
}
#pragma endregion

#pragma region Sprite
Sprite::Sprite(String texture, Rectangle source, Vector4 color, Rectangle destination)
{
    m_texture = Services::GetService<ContentSystem>()->GetTexture(texture);

    m_color = color;
    m_destination = destination;
    m_source = source;
}

Sprite::~Sprite()
{
    m_texture = nullptr;
}

void Sprite::Update(float elapsedTime)
{

}

void Sprite::Draw()
{
    if (!m_active) return;

    m_spritebatch->Draw(
        m_texture,      // ID3D11ShaderResourceView*
        m_destination,  // const RECT&
        &m_source,      // const RECT*
        m_color,        // FXMVECTOR (Colors::White)
        m_rotation,     // float (0)
        m_origin,       // const XMFLOAT2& (Float2Zero)
        m_flip,         // SpriteEffects (SpriteEffects_None)
        m_depth         // float (0)
    );
}
#pragma endregion

#pragma region SpriteStrip
SpriteStrip::SpriteStrip(TextureComPtr texture, Rectangle source, unsigned frameCount, bool hasStatic)
    : m_texture{ texture }
    , m_source{ source }
    , m_frame_count{ frameCount }
    , m_has_static{ hasStatic }
{
    for (auto i = 0ul; i <= frameCount; ++i)
    {
        RECT r;
        r.left = m_source.x + m_source.width * i;
        r.top = m_source.y;
        r.right = r.left + m_source.width;
        r.bottom = r.top + m_source.height;

        if (i < m_frame_count || (i == frameCount && hasStatic))
        {
            m_frames.push_back(r);
        }
    }
}

SpriteStrip::~SpriteStrip()
{
    m_texture = nullptr;
}

void SpriteStrip::Update(float elapsedTime)
{
    if (!m_active || m_animation_paused) return;

    m_frame_elapsed_time += elapsedTime;

    if (m_frame_elapsed_time >= m_time_per_frame)
    {
        if (m_draw_static)
        {
            m_current_frame = m_has_static ? m_frame_count : 0;
        }
        else
        {
            m_current_frame = (m_current_frame + 1) % m_frame_count;
        }

        m_frame_elapsed_time = 0.f;
    }
}

void SpriteStrip::Draw()
{
    if (!m_active) return;

    m_spritebatch->Draw(
        m_texture.Get(),
        GetPosition(),
        &m_frames[m_current_frame],
        m_color,
        m_rotation,
        m_origin,
        m_scale,
        m_flip,
        m_depth
    );
}
#pragma endregion

#pragma region SpriteString
SpriteString::SpriteString(SpriteFontRaw spriteFont, Unicode string)
    : m_spritefont{ spriteFont }
    , m_string{ string }
{

}

SpriteString::~SpriteString()
{
    m_spritefont = nullptr;
}

void SpriteString::Update(float elapsedTime)
{

}

void SpriteString::Draw()
{
    if (!m_active) return;
        
    m_spritefont->DrawString(
        m_spritebatch,
        m_string.c_str(),
        GetPosition(),
        m_color,
        m_rotation,
        m_origin,
        m_scale,
        m_flip,
        m_depth
    );
}
#pragma endregion
