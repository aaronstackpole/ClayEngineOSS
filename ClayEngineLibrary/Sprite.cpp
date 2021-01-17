#include "pch.h"

#include "Sprite.h"

using namespace DirectX;

namespace ClayEngine
{
	bool _Resource_is_texture(TexturePtr texture) noexcept
	{
		if (!texture) return false;

		ResourcePtr res;
		texture->GetResource(res.GetAddressOf());

		D3D11_RESOURCE_DIMENSION dim;
		res->GetType(&dim);

		return (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D);
	}
}

#pragma region SpriteBase
ClayEngine::SpriteBase::SpriteBase()
	: is_active{ false }
	, position{ 0.f, 0.f }
	, origin{ 0.f, 0.f }
	, offset{ 0.f, 0.f }
	, scale{ 1.f }
	, rotation{ 0.f }
	, depth{ 0.f }
	, alpha{ Vector4(1.f) }
{
}

void ClayEngine::SpriteBase::SetActive(bool active)
{
	is_active = active;
}

void ClayEngine::SpriteBase::SetPosition(float x, float y)
{
	position = Vector2{ x, y };
}

void ClayEngine::SpriteBase::SetPosition(Vector2 _position)
{
	position = _position;
}

void ClayEngine::SpriteBase::SetOrigin(float x, float y)
{
	origin = Vector2{ x, y };
}

void ClayEngine::SpriteBase::SetOrigin(Vector2 _origin)
{
	origin = _origin;
}

void ClayEngine::SpriteBase::SetOffset(float x, float y)
{
	offset = Vector2{ x, y };
}

void ClayEngine::SpriteBase::SetOffset(Vector2 _offset)
{
	offset = _offset;
}

void ClayEngine::SpriteBase::SetScale(float _scale)
{
	scale = Vector2{ _scale, _scale };
}

void ClayEngine::SpriteBase::SetScale(float x, float y)
{
	scale = Vector2{ x, y };
}

void ClayEngine::SpriteBase::SetScale(Vector2 _scale)
{
	scale = _scale;
}

void ClayEngine::SpriteBase::SetRotation(float _rotation)
{
	rotation = _rotation;
}

void ClayEngine::SpriteBase::SetDepth(float _depth)
{
	depth = _depth;
}

void ClayEngine::SpriteBase::SetAlpha(float _alpha)
{
	alpha = Vector4{ 0.f, 0.f, 0.f, _alpha };
}
#pragma endregion

#pragma region SpriteStrip
ClayEngine::SpriteStrip::SpriteStrip(TexturePtr texturePtr, Rectangle sourceRect, uint32_t frameCount, bool hasStatic)
	: texture{ texturePtr }
	, source{ sourceRect }
	, frame_count{ frameCount }
	, has_static{ hasStatic }
	, is_paused{ true }
{
	assert(_Resource_is_texture(texturePtr));

	frame_elapsed_time = 0.f;
	current_frame = 0;
	time_per_frame = 0.1f; // Hard coded 10 FPS

	for (auto i = 0u; i <= frameCount; ++i)
	{
		RECT r;
		r.left = source.x + source.width * i;
		r.top = source.y;
		r.right = r.left + source.width;
		r.bottom = r.top + source.height;

		if (i < frame_count || (i == frameCount && hasStatic))
		{
			frames.push_back(r);
		}
	}
}

void ClayEngine::SpriteStrip::SetActive(bool isActive)
{
	is_active = isActive;
}

void ClayEngine::SpriteStrip::SetPaused(bool isPaused)
{
	is_paused = isPaused;
}

void ClayEngine::SpriteStrip::SetStatic(bool hasStatic)
{
	has_static = hasStatic;
}

void ClayEngine::SpriteStrip::SetFramerate(float fps)
{
	time_per_frame = 1.f / fps;
}

void ClayEngine::SpriteStrip::Update(float elapsedTime)
{
	if (!is_active) return;

	if (has_static)
	{
		current_frame = has_static_frame ? frame_count : 0;
		return;
	}

	if (is_active && !is_paused)
	{
		frame_elapsed_time += elapsedTime;

		if (frame_elapsed_time >= time_per_frame)
		{
			current_frame = (current_frame + 1) % frame_count;
			frame_elapsed_time = 0.f;
		}
	}
}

void ClayEngine::SpriteStrip::Draw(SpriteBatch* spriteBatch)
{
	assert(spriteBatch);

	if (is_active)
	{
		spriteBatch->Draw(
			texture.Get(),
			position,
			&frames[current_frame],
			alpha,
			rotation,
			offset,
			scale,
			SpriteEffects::SpriteEffects_None,
			depth
		);
	}

}
#pragma endregion

#pragma region SpriteString
ClayEngine::SpriteString::SpriteString(SpriteFont* spriteFont, String initialString)
	: font{ spriteFont }
	, string{ initialString }
{
}

void ClayEngine::SpriteString::SetFont(SpriteFont* fontPtr)
{
	font = fontPtr;
}

void ClayEngine::SpriteString::SetString(String newString)
{
	string = newString;
}

void ClayEngine::SpriteString::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	if (!is_active) return;

	//m_origin = (m_font->MeasureString(m_string.c_str()) * 0.5F);
}

void ClayEngine::SpriteString::Draw(SpriteBatch* spriteBatch)
{
	assert(spriteBatch);

	if (is_active)
	{
		font->DrawString(
			spriteBatch,
			string.c_str(),
			position,
			Colors::White,
			rotation,
			origin,
			scale,
			SpriteEffects::SpriteEffects_None,
			depth
		);
	}
}
#pragma endregion