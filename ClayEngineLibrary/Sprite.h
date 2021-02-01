#pragma once

#include "ClayEngine.h"

#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "SimpleMath.h"

_CE_BEGIN
using TexturePtr = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
using ResourcePtr = Microsoft::WRL::ComPtr<ID3D11Resource>;
using SpriteBatchPtr = DirectX::SpriteBatch*;
using SpriteFontPtr = DirectX::SpriteFont*;
using Rect = DirectX::SimpleMath::Rectangle;
using Frames = std::vector<RECT>;

struct ISprite
{
	virtual ISprite* Clone() const = 0;
	virtual void Update(float elapsedTime) = 0;
	virtual void Draw(SpriteBatchPtr spriteBatch) = 0;
};

class SpriteBase
{
protected:
	bool is_active;
	DirectX::SimpleMath::Vector2 position;
	DirectX::SimpleMath::Vector2 origin;
	DirectX::SimpleMath::Vector2 offset;
	DirectX::SimpleMath::Vector2 scale;
	float rotation;
	float depth;
	DirectX::SimpleMath::Vector4 alpha;

public:
	SpriteBase();
	virtual ~SpriteBase() = default;

	virtual void SetActive(bool active);

	void SetPosition(float x, float y);
	void SetPosition(DirectX::SimpleMath::Vector2 position);
	void SetOrigin(float x, float y);
	void SetOrigin(DirectX::SimpleMath::Vector2 origin);
	void SetOffset(float x, float y);
	void SetOffset(DirectX::SimpleMath::Vector2 offset);
	void SetScale(float scale);
	void SetScale(float x, float y);
	void SetScale(DirectX::SimpleMath::Vector2 scale);
	void SetRotation(float rotation);
	void SetDepth(float depth);
	void SetAlpha(float alpha);
};


class SpriteStrip : public SpriteBase, ISprite
{
	TexturePtr texture{};
	Rect source{};
	Frames frames{};

	double frame_elapsed_time;
	uint32_t current_frame;
	double time_per_frame;
	uint32_t frame_count;
	bool has_static_frame;

	bool is_paused;
	bool has_static;

public:
	SpriteStrip(TexturePtr texturePtr, Rect sourceRect, uint32_t frameCount, bool hasStatic);

	virtual void SetActive(bool active) override;

	void SetPaused(bool isPaused);
	void SetStatic(bool hasStatic);
	void SetFramerate(float fps);

	virtual ISprite* Clone() const override { return new SpriteStrip(*this); }
	virtual void Update(float elapsedTime) override;
	virtual void Draw(SpriteBatchPtr spriteBatch) override;
};


class SpriteString : public SpriteBase, ISprite
{
	SpriteFontPtr font{};
	String string{};

public:
	SpriteString(SpriteFontPtr spriteFont, String initialString);
	
	void SetFont(SpriteFontPtr fontPtr);
	void SetString(String newString);

	virtual ISprite* Clone() const override { return new SpriteString(*this); }
	virtual void Update(float elapsedTime) override;
	virtual void Draw(SpriteBatchPtr spriteBatch) override;
};
_CE_END