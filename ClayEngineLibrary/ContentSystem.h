#pragma once

#include "ClayEngine.h"

#include <map>

#include <SpriteFont.h>
#include <WICTextureLoader.h>

namespace ClayEngine
{
    class FontRegistry
    {
        using SpriteFontPtr = std::unique_ptr<DirectX::SpriteFont>;
        using SpriteFontRaw = DirectX::SpriteFont*;

        ID3D11Device* dev;

    public:
        enum class FontFace { Fixed, Serif, SansSerif };
        enum class FontSize { Small, Medium, Large, ExtraLarge };

        FontRegistry(ID3D11Device* device)
            : dev{ device }
        {
        }
        ~FontRegistry() = default;

        void AddFont(FontFace face, FontSize size, String path);
        SpriteFontRaw GetFontPtr(FontFace face, FontSize size);


    private:
        using FixedFonts = std::map<FontRegistry::FontSize, SpriteFontPtr>;
        using SerifFonts = std::map<FontRegistry::FontSize, SpriteFontPtr>;
        using SansSerifFonts = std::map<FontRegistry::FontSize, SpriteFontPtr>;

        FixedFonts fixed_fonts;
        SerifFonts serif_fonts;
        SansSerifFonts sans_fonts;
    };
    using FontRegistryPtr = std::unique_ptr<FontRegistry>;

    class TextureRegistry
    {
        using TexturePtr = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
        using TextureRaw = ID3D11ShaderResourceView*;

        ID3D11Device* dev;

    public:
        TextureRegistry(ID3D11Device* device)
            : dev{ device }
        {
        }
        ~TextureRegistry() = default;

        void AddTexture(String key, String path);
        TextureRaw GetTexturePtr(String key);


    private:
        using Textures = std::map<String, TexturePtr>;

        Textures textures;
    };
    using TextureRegistryPtr = std::unique_ptr<TextureRegistry>;

    class ContentSystem
    {
        FontRegistryPtr fonts;
        TextureRegistryPtr textures;

    public:
        ContentSystem(ID3D11Device* device)
        {
            fonts = std::make_unique<FontRegistry>(device);
            textures = std::make_unique<TextureRegistry>(device);

            // At this point, we need to get from the filesystem a list of font and texture files
            // that we're going to load, we then need to queue that list up for a worker thread
            // to asynchronously load all of the content.
            // We should play some kind of loading screen during this time, so it might be useful to have
            // some very fast initial graphics loaded for a progress bar, for example. A font and a texture
            // hard coded to be added right after instantiation, for example.

        }
        ~ContentSystem()
        {
            textures.reset();
            fonts.reset();
        }
    };
    using ContentSystemPtr = std::unique_ptr<ContentSystem>;
}