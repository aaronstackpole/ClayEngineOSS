#include "pch.h"
#include "Settings.h"

using namespace ClayEngine;
using namespace ClayEngine::Platform;

Settings::Settings()
{
    m_json = std::make_unique<JsonFile>(c_settings_json);
    auto document = m_json->GetDocument();

    m_settings.Width = document["video"]["width"];
    m_settings.Height = document["video"]["height"];

    m_settings.MoveForward = document["control"]["move_forward"].get<std::string>()[0];
    m_settings.MoveLeft = document["control"]["move_left"].get<std::string>()[0];
    m_settings.MoveBackward = document["control"]["move_backward"].get<std::string>()[0];
    m_settings.MoveRight = document["control"]["move_right"].get<std::string>()[0];
}

Settings::~Settings()
{
    m_json.reset();
    m_json = nullptr;
}

const ClayEngineSettings& Settings::GetWindowSettings() const
{
    return m_settings;
}
