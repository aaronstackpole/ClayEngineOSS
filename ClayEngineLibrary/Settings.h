#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Settings Class Library (C) 2022 Epoch Meridian, LLC.            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "Storage.h"

namespace ClayEngine
{
	constexpr auto c_settings_json = "clayengine.json";

	constexpr auto c_default_window_width = 1920U;
	constexpr auto c_default_window_height = 1080U;

	/// <summary>
	/// A POC data class for some game settings such as default movement key bindings and screen resolution
	/// </summary>
	struct ClayEngineSettings
	{
		int Width = c_default_window_width; // Screen width (in pixels) to request
		int Height = c_default_window_height; // Screen height (in pixels) to request

		char MoveForward = 0;
		char MoveLeft = 0;
		char MoveBackward = 0;
		char MoveRight = 0;
	};

	/// <summary>
	/// This class provides an interface to settings stored on disk
	/// </summary>
	class Settings
	{
		ClayEngine::Platform::JsonFilePtr m_json = nullptr;
		ClayEngineSettings m_settings = {};

	public:
		Settings() noexcept(false);
		Settings(Settings const&) = delete;
		Settings& operator=(Settings const&) = delete;
		Settings(Settings&&) = default;
		Settings& operator=(Settings&&) = default;
		~Settings();

		const ClayEngineSettings& GetWindowSettings() const;
	};
	using SettingsPtr = std::unique_ptr<Settings>;
	using SettingsRaw = Settings*;

	//class CommandParser
	//{
	//public:
	//	enum class Verbs
	//	{
	//		Invalid,
	//		Set,
	//	};
	//	enum class Nouns
	//	{
	//		Invalid,
	//		ScreenWidth,
	//		ScreenHeight,
	//	};
	//
	//private:
	//	Verbs verb = Verbs::Invalid;
	//	Nouns noun = Nouns::Invalid;
	//	int value = 0;
	//
	//
	//public:
	//	CommandParser() = default;
	//	~CommandParser() = default;
	//
	//	// Command Parser will execute the command in the string provided
	//	void ParseCommand(std::string line)
	//	{
	//		return;
	//	}
	//};
}

