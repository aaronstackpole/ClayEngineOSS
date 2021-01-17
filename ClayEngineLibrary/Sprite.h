#pragma once

#include "ClayEngine.h"

#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "SimpleMath.h"

_CE_BEGIN
using TexturePtr = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
using ResourcePtr = Microsoft::WRL::ComPtr<ID3D11Resource>;
using Frames = std::vector<RECT>;
using DirectX::SpriteBatch;
using DirectX::SpriteFont;
using DirectX::SimpleMath::Rectangle;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector4;

struct ISprite
{
	virtual ISprite* Clone() const = 0;
	virtual void Update(float elapsedTime) = 0;
	virtual void Draw(SpriteBatch* spriteBatch) = 0;
};

class SpriteBase
{
protected:
	bool is_active;
	Vector2 position;
	Vector2 origin;
	Vector2 offset;
	Vector2 scale;
	float rotation;
	float depth;
	Vector4 alpha;

public:
	SpriteBase();
	virtual ~SpriteBase() = default;

	virtual void SetActive(bool active);

	void SetPosition(float x, float y);
	void SetPosition(Vector2 position);
	void SetOrigin(float x, float y);
	void SetOrigin(Vector2 origin);
	void SetOffset(float x, float y);
	void SetOffset(Vector2 offset);
	void SetScale(float scale);
	void SetScale(float x, float y);
	void SetScale(Vector2 scale);
	void SetRotation(float rotation);
	void SetDepth(float depth);
	void SetAlpha(float alpha);
};


class SpriteStrip : public SpriteBase, ISprite
{
	TexturePtr texture{};
	Rectangle source{};
	Frames frames{};

	double frame_elapsed_time;
	uint32_t current_frame;
	double time_per_frame;
	uint32_t frame_count;
	bool has_static_frame;

	bool is_paused;
	bool has_static;

public:
	SpriteStrip(TexturePtr texturePtr, Rectangle sourceRect, uint32_t frameCount, bool hasStatic);

	virtual void SetActive(bool active) override;

	void SetPaused(bool isPaused);
	void SetStatic(bool hasStatic);
	void SetFramerate(float fps);

	virtual ISprite* Clone() const override { return new SpriteStrip(*this); }
	virtual void Update(float elapsedTime) override;
	virtual void Draw(SpriteBatch* spriteBatch) override;
};


class SpriteString : public SpriteBase, ISprite
{
	SpriteFont* font{};
	String string{};

public:
	SpriteString(SpriteFont* spriteFont, String initialString);
	
	void SetFont(SpriteFont* fontPtr);
	void SetString(String newString);

	virtual ISprite* Clone() const override { return new SpriteString(*this); }
	virtual void Update(float elapsedTime) override;
	virtual void Draw(SpriteBatch* spriteBatch) override;
};
_CE_END