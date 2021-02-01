#include "pch.h"
#include "ContentSystem.h"

void ClayEngine::FontRegistry::AddFont(FontFace face, FontSize size, String path)
{
    switch (face)
    {
    case FontFace::Fixed:
        fixed_fonts.emplace(size, std::make_unique<DirectX::SpriteFont>(dev, path.c_str()));
        break;
    case FontFace::Serif:
        serif_fonts.emplace(size, std::make_unique<DirectX::SpriteFont>(dev, path.c_str()));
        break;
    case FontFace::SansSerif:
        sans_fonts.emplace(size, std::make_unique<DirectX::SpriteFont>(dev, path.c_str()));
        break;
    }
    throw;
}

ClayEngine::FontRegistry::SpriteFontRaw ClayEngine::FontRegistry::GetFontPtr(FontFace face, FontSize size)
{
    switch (face)
    {
    case FontFace::Fixed:
        return fixed_fonts.at(size).get();
    case FontFace::Serif:
        return serif_fonts.at(size).get();
    case FontFace::SansSerif:
        return sans_fonts.at(size).get();
    }
    throw;
}

void ClayEngine::TextureRegistry::AddTexture(String key, String path)
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    auto hr = DirectX::CreateWICTextureFromFile(dev, path.c_str(), nullptr, texture.ReleaseAndGetAddressOf());
    if (!FAILED(hr))
        textures.emplace(key, texture);

    throw;
}

ClayEngine::TextureRegistry::TextureRaw ClayEngine::TextureRegistry::GetTexturePtr(String key)
{
    auto it = textures.find(key);
    if (it != textures.end())
        return textures.at(key).Get();
    
    throw;
}
