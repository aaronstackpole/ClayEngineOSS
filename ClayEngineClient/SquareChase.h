#pragma once

#include <vector>
#include <memory>

#include "SimpleMath.h"

#include "ClayEngine.h"
#include "WindowSystem.h"
#include "InputSystem.h"
#include "RenderSystem.h"
#include "ContentSystem.h"
#include "TimingSystem.h"

#include "Sprite.h"

namespace ClayEngine
{
	namespace Game
	{
		using namespace ClayEngine;
		using namespace ClayEngine::Graphics;
		using namespace ClayEngine::Platform;
		using namespace ClayEngine::SimpleMath;
		using namespace DirectX::SimpleMath;

		class SquareChase
		{
			InputSystemRaw m_is = nullptr;
			ContentSystemRaw m_cs = nullptr;
			SpritePtr m_sprite = nullptr;
			SpriteStringPtr m_display_x = nullptr;
			SpriteStringPtr m_display_y = nullptr;
			SpriteStringPtr m_display_score = nullptr;
			SpriteStringPtr m_display_speed = nullptr;

			Rectangle m_current_square;

			float m_time_remaining = 0.f;

			int m_player_score = 0;
			int m_time_per_square = 300;

			using Colors = std::vector<Vector4>;
			Colors m_colors = { Vector4{ 1.f, 0.f, 0.f, 1.f }, Vector4{ 0.f, 0.5f, 0.f, 1.f }, Vector4{ 0.f, 0.f, 1.f, 1.f } };

		public:
			SquareChase()
			{
				m_is = Services::GetService<InputSystem>();
				m_cs = Services::GetService<ContentSystem>();

				auto ticker = Services::GetService<TimingSystem>();
				auto w = WindowSystem::GetWindowWidth();
				auto h = WindowSystem::GetWindowHeight();

				m_sprite = std::make_unique<Sprite>("Pixel", Rectangle{ 0, 0, 1, 1 }, Vector4{ 1.f, 1.f, 1.f, 1.f });
				ticker->AddDrawCallback([&]() { m_sprite->Draw(); });

				m_display_x = std::make_unique<SpriteString>(m_cs->GetFont("Consolas_10"));
				m_display_x->SetPosition(float(w - 100), float(h - 30));
				ticker->AddDrawCallback([&]() {m_display_x->Draw(); });

				m_display_y = std::make_unique<SpriteString>(m_cs->GetFont("Consolas_10"));
				m_display_y->SetPosition(float(w - 100), float(h - 20));
				ticker->AddDrawCallback([&]() {m_display_y->Draw(); });

				m_display_score = std::make_unique<SpriteString>(m_cs->GetFont("Consolas_16"), L"Score: ");
				m_display_score->SetPosition(float(w - 158), float(h - 54));
				m_display_score->SetRGBA(1.f, 0.1f, 0.1f, 1.f);
				ticker->AddDrawCallback([&]() {m_display_score->Draw(); });

				m_display_speed = std::make_unique<SpriteString>(m_cs->GetFont("Consolas_16"), L"Time: ");
				m_display_speed->SetPosition(float(w - 145), float(h - 78));
				m_display_speed->SetRGBA(1.f, 0.1f, 0.1f, 1.f);
				ticker->AddDrawCallback([&]() {m_display_speed->Draw(); });

			}
			~SquareChase()
			{
				m_sprite.reset();
				m_sprite = nullptr;

				m_is = nullptr;
			}

			void Update(float elapsedTime)
			{
				if (m_time_remaining <= 0.f)
				{
					auto x = GetNextInt(0, WindowSystem::GetWindowWidth() - 64);
					std::wstringstream  wssx;
					wssx << L"Rect X: " << x;
					m_display_x->SetString(wssx.str());

					auto y = GetNextInt(0, WindowSystem::GetWindowHeight() - 64);
					std::wstringstream  wssy;
					wssy << L"Rect Y: " << y;
					m_display_y->SetString(wssy.str());

					m_current_square = Rectangle{ x, y, 64, 64 };
					m_sprite->SetDestination(m_current_square);
					m_sprite->SetRGBA(m_colors[m_player_score % 3]);
					m_time_remaining = float(m_time_per_square);
				}

				m_time_remaining -= elapsedTime;

				auto m = m_is->GetMouseState();
				if (m.leftButton)
				{
					if (m_current_square.Contains(m.x, m.y))
					{
						m_player_score++;
						m_time_per_square--;
						m_time_remaining = 0.f;

						//<< " Time: " << m_time_per_square;
						std::wstringstream wsss;
						wsss << "Score: " << m_player_score;
						m_display_score->SetString(wsss.str());

						std::wstringstream wsst;
						wsst << "Time: " << m_time_per_square << L"ms";
						m_display_speed->SetString(wsst.str());
					}
				}
			}
		};
		using SquareChasePtr = std::unique_ptr<SquareChase>;
	}
}
