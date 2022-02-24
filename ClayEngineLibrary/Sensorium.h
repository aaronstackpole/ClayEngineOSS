#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine User Interface Class Library (C) 2022 Epoch Meridian, LLC.      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include <array>
#include <memory>
#include <vector>

#include "SpriteBatch.h"
#include "Mouse.h"

#include "ClayEngine.h"
#include "InputSystem.h"
#include "ContentSystem.h"
#include "TimingSystem.h"
#include "WindowSystem.h"
#include "Sprite.h"

namespace ClayEngine
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;
	using namespace ClayEngine::Graphics;
	using namespace ClayEngine::Platform;
	using namespace ClayEngine::SimpleMath;

	namespace Game
	{
#pragma region Tree Element Classes
		/// <summary>
		/// A generic tree element system, each element holds one Sprite object
		/// </summary>
		class TreeElement;
		using TreeElementPtr = std::unique_ptr<TreeElement>;
		using TreeElementRaw = TreeElement*;
		using TreeElements = std::vector<TreeElementRaw>; // Quuestionable ambiguity
		class TreeElement
		{
			using Children = std::vector<TreeElementPtr>;
			Children c = {};
			TreeElementRaw p = nullptr;
			String k = {};
			SpriteRaw o = nullptr;
		public:
			TreeElement(TreeElementRaw parent, String key, SpriteRaw sprite)
			{
				k = key;
				o = sprite;
				p = parent;
			}
			~TreeElement()
			{
				p = nullptr;
				o = nullptr;
			}
			const String& GetKey() { return k; }
			SpriteRaw GetObject() { return o; }
			TreeElementRaw GetParent() { return p; }
			TreeElementRaw GetRoot()
			{
				auto r = GetParent();
				while (r != nullptr)
				{
					r->GetParent();
				}
				return this;
			}
			void AddChild(String key, SpriteRaw sprite)
			{
				c.emplace_back(std::make_unique<TreeElement>(this, key, sprite));
			}
			TreeElements GetChildren()
			{
				TreeElements v = {};

				if (c.size() > 0)
				{
					for (auto& outer : c)
					{
						auto ci = outer->GetChildren();
						if (ci.size() > 0)
						{
							for (auto inner : ci)
							{
								v.push_back(inner);
							}
						}
						v.push_back(outer.get());
					}
				}
				return v;
			}
			void Update()
			{
				GetObject()->SetPosition(p->GetObject()->GetPosition());
			}
		};
#pragma endregion

		namespace Extensions
		{
			/// <summary>
			/// Provides an inner and outer rectangle for nesting windows
			/// </summary>
			struct UIElementNestingWindowExtension
			{
			protected:
				Rectangle m_outerbox = {};
				Rectangle m_innerbox = {};

			public:
				Rectangle GetInnerRectangle()
				{
					return Rectangle(m_innerbox);
				}
				Rectangle GetOuterRectangle()
				{
					return Rectangle(m_outerbox);
				}
			};

			/// <summary>
			/// Provides an element mask value used for frame click resize logic
			/// </summary>
			struct UIElementMaskExtension
			{
			protected:
				UINT m_mask = 0x00;

			public:
				void SetMask(UINT mask) { m_mask = mask; }
				UINT GetMask() { return m_mask; }
			};
		}

		namespace Interfaces
		{
			struct IUpdate
			{
				virtual void Update(float elapsedTime) = 0;
			};

			struct IDraw
			{
				virtual void Draw() = 0;
			};

			struct IUIElement : public IUpdate, public IDraw
			{
				virtual void OnLayoutChanged() = 0;
			};
		}

		/// <summary>
		/// A pool of SpriteString elements that can be used to display text in a text box
		/// </summary>
		class SpriteStringPool
		{
			using SpriteStrings = std::vector<SpriteString>;
			SpriteStrings m_strings = {};

		public:
			SpriteStringPool()
			{
				m_strings.reserve(20);
			}
			~SpriteStringPool()
			{

			}

			int GetFontHeight()
			{
				return 24;
			}
		};
		using SpriteStringPoolPtr = std::unique_ptr<SpriteStringPool>;

		/// <summary>
		/// An element that displays text lines from <TODO> a text buffer system...
		/// </summary>
		class TextWindow
		{
			long m_border = 0l;

		public:
			TextWindow()
			{

			}
			~TextWindow()
			{

			}
		};
		using TextWindowPtr = std::unique_ptr<TextWindow>;

		/// <summary>
		/// An element that provides scroll control and scales based on text box content
		/// </summary>
		class ScrollBar
		{
			//TODO: "Magic" number 2l is not aware of the scaling (50%) done to the sprite source image

			SpritePtr m_scroll_up = nullptr;
			SpritePtr m_scroll_tray = nullptr;
			SpritePtr m_scroll_down = nullptr;
			SpritePtr m_slider_top = nullptr;
			SpritePtr m_slider_mid = nullptr;
			SpritePtr m_slider_bot = nullptr;

			long m_control_size = 32l;
			float m_scrollbar_size = 0.9f;
			bool m_active = false;

			// How many lines displayable?
			// How many lines total?

			// When total is less than displayable lines, bar is inactive

			// When total exceeds displayable lines, bar will be:
			// Bar Max scrollable size 90% (1 line)
			// Bar Min scrollable size 10% (200 lines)

			// Line wrap should be calculated and considered

		public:
			ScrollBar(DirectX::SimpleMath::Rectangle destination)
			{
				auto ticker = Services::GetService<TimingSystem>();

				m_scroll_up = std::make_unique<Sprite>("UIElements", Rectangle{ 0, 0, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.f }, Rectangle{ destination.x, destination.y, m_control_size, m_control_size });
				ticker->AddDrawCallback([&]() { m_scroll_up->Draw(); });

				m_scroll_tray = std::make_unique<Sprite>("Pixel", Rectangle{ 0, 0, 1, 1 }, Vector4{ 0.f, 0.f, 0.f, 1.f }, Rectangle{ destination.x, destination.y + m_control_size, m_control_size, destination.y + destination.height - (m_control_size * 2) });
				ticker->AddDrawCallback([&]() { m_scroll_tray->Draw(); });

				m_scroll_down = std::make_unique<Sprite>("UIElements", Rectangle{ 0, 64, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.f }, Rectangle{ destination.x, destination.y + destination.height - m_control_size, m_control_size, m_control_size });
				ticker->AddDrawCallback([&]() { m_scroll_down->Draw(); });

				m_slider_top = std::make_unique<Sprite>("UIElements", Rectangle{ 0, 0, 64, 4 }, Vector4{ 1.f, 1.f, 1.f, 1.f }, Rectangle{ destination.x, destination.y, m_control_size, 2l });
				ticker->AddDrawCallback([&]() { m_slider_top->Draw(); });

				m_slider_mid = std::make_unique<Sprite>("UIElements", Rectangle{ 0, 4, 64, 4 }, Vector4{ 1.f, 1.f, 1.f, 1.f }, Rectangle{ destination.x, destination.y, m_control_size, 2l });
				ticker->AddDrawCallback([&]() { m_slider_mid->Draw(); });

				m_slider_bot = std::make_unique<Sprite>("UIElements", Rectangle{ 0, 60, 64, 4 }, Vector4{ 1.f, 1.f, 1.f, 1.f }, Rectangle{ destination.x, destination.y, m_control_size, 2l });
				ticker->AddDrawCallback([&]() { m_slider_bot->Draw(); });
			}
			~ScrollBar()
			{

			}

			void SetActive(bool active)
			{
				m_active = active;
			}
			void SetSliderSize(float percent)
			{
				float bounded = percent;
				if (percent > 0.9f) bounded = 0.9f;
				if (percent < 0.1f) bounded = 0.1f;

				m_scrollbar_size = bounded;
			}
			void SetSliderPosition(float position)
			{
				// Calculate the center point where the slider should be
				// This is based on total number of rows over vs position in the scrollback
			}
			void SetDestination(long x, long y, long width, long height)
			{
				m_scroll_up->SetDestination(x, y, m_control_size, m_control_size);
				m_scroll_tray->SetDestination(x, y + m_control_size, m_control_size, height - (m_control_size * 2));
				m_scroll_down->SetDestination(x, y + height - m_control_size, m_control_size, m_control_size);


				// scroll_bar will take some extra logic...
				if (m_active)
				{

				}
				else
				{
					m_slider_top->SetDestination(x, y + m_control_size, m_control_size, 2l);
					m_slider_mid->SetDestination(x, y + m_control_size + 2l, m_control_size, height - (m_control_size * 2) - (2l * 2));
					m_slider_bot->SetDestination(x, y + height - m_control_size - 2l, m_control_size, 2l);
				}
			}
			void SetDestination(DirectX::SimpleMath::Rectangle destination)
			{
				SetDestination(destination.x, destination.y, destination.width, destination.height);
			}
		};
		using ScrollBarPtr = std::unique_ptr<ScrollBar>;

		/// <summary>
		/// An element that displays input text with cursor, highlight, and edit display capability
		/// </summary>
		class InputBar
		{
			InputSystemRaw m_is = nullptr;

			long m_border = 0l;

			SpriteStringPtr m_text = nullptr;
			SpriteStringPtr m_cursor = nullptr;

			Rectangle m_destination = {};

			float m_elapsed = 0.f;
			bool m_cursor_visible = true;

			// Background Texture
			// Insert/Select square Texture

		public:
			InputBar()
			{
				auto ticker = Services::GetService<TimingSystem>();
				m_is = Services::GetService<InputSystem>();

				m_text = std::make_unique<SpriteString>(Services::GetService<ContentSystem>()->GetFont("Consolas_24"));
				m_text->SetString(L"DERP");
				ticker->AddDrawCallback([&]() { m_text->Draw(); });

				m_cursor = std::make_unique<SpriteString>(Services::GetService<ContentSystem>()->GetFont("Consolas_24"));
				m_cursor->SetString(L"_");
				ticker->AddDrawCallback([&]() { m_cursor->Draw(); });
			}
			~InputBar()
			{
				m_is = nullptr;

				m_cursor.reset();
				m_cursor = nullptr;

				m_text.reset();
				m_text = nullptr;
			}

			void SetDestination(long x, long y, long width, long height)
			{
				m_destination.x = x;
				m_destination.y = y;
				m_destination.width = width;
				m_destination.height = height;

				//m_sprite->SetDestination(x, y, width, height);

				m_text->SetPosition(x, y);
				m_cursor->SetPosition(x, y);
			}
			void SetDestination(Rectangle destination)
			{
				SetDestination(destination.x, destination.y, destination.width, destination.height);
			}

			void Update(float elapsedTime)
			{
				m_text->SetString(m_is->GetBuffer());
				auto p = m_text->GetPosition();
				//m_text->SetPostiion();

				m_cursor->SetPosition(p.x + (m_is->GetCaratIdx() * 18.f), p.y);

				m_elapsed += elapsedTime;
				if (m_elapsed >= 250.f)
				{
					m_elapsed = 0.f;
					m_cursor_visible = !m_cursor_visible;
					m_cursor->SetActive(m_cursor_visible);
				}
			}

		};
		using InputBarPtr = std::unique_ptr<InputBar>;

		/// <summary>
		/// An element that contains an input bar and a text buffer rendering window
		/// </summary>
		class InputWindow
		{
			long m_border = 0l;
			long m_control_size = 32l;

			bool m_show_inputbar = true;

			InputBarPtr m_inputbar = nullptr;
			TextWindowPtr m_textwindow = nullptr;

			Rectangle m_outer = {};
			Rectangle m_inner = {};

		public:
			InputWindow()
			{

			}
			~InputWindow()
			{
				if (m_textwindow)
				{
					m_textwindow.reset();
					m_textwindow = nullptr;
				}

				if (m_inputbar)
				{
					m_inputbar.reset();
					m_inputbar = nullptr;
				}
			}
		};
		using InputWindowPtr = std::unique_ptr<InputWindow>;

		/// <summary>
		/// Window contains a scrollbar element to feed text window parameters
		/// </summary>
		class ScrollableWindow
		{
			long m_border = 1l;
			long m_control_size = 32l;

			bool m_show_scrollbar = true;

			ScrollBarPtr m_scrollbar = nullptr;
			InputWindowPtr m_inputwindow = nullptr;

			Rectangle m_outer = {};
			Rectangle m_inner = {};

		public:
			ScrollableWindow()
			{

			}
			~ScrollableWindow()
			{
				if (m_inputwindow)
				{
					m_inputwindow.reset();
					m_inputwindow = nullptr;
				}

				if (m_scrollbar)
				{
					m_scrollbar.reset();
					m_scrollbar = nullptr;
				}
			}

			void TempScollbarCalcCode(Rectangle destination)
			{
				//TODO: Get details about text from textbox and provide to scrollbar
				//auto font_height = m_textbox->GetFontHeight();
				//auto buffer_size = m_is->GetBackBufferSize();
				//auto text_height = buffer_size * font_height;
				//if (text_height <= destination.height)
				//{
					// All good, render them all...
				//}
				//else
				//{
					// Lines greater than frame, calculate by how much...
					//auto over = destination.height - text_height;
					//int lines_over = 0;
					//while (over >= font_height)
					//{
						//++lines_over;
						//over -= font_height;
					//}
				//}
				// 
				//auto overage = over / buffer_size; // This is the amount to subtract from the scroll bar 100%
				// We've got our overage % now tell the scrollbar what size to be

				//if (m_show_scrollbar)
				//{
				//	if (!m_scrollbar)
				//	{
				//		m_scrollbar = std::make_unique<ScrollBar>(DirectX::SimpleMath::Rectangle{ 0,0,0,0 });
				//	}

				//	 Update m_inner!
				//}
				//else
				//{
				//	if (m_scrollbar)
				//	{
				//		m_scrollbar.reset();
				//		m_scrollbar = nullptr;
				//	}

				//	 Update m_inner?
				//}
			}
		};
		using ScrollableWindowPtr = std::unique_ptr<ScrollableWindow>;

		/// <summary>
		/// An interactive component that can be used to callback a value between 0.f and 1.f
		/// </summary>
		class Slider
		{
			InputSystemRaw m_is = nullptr;

			using FloatCallback = std::function<void(float)>;
			FloatCallback m_fn = {};

			Rectangle m_destination = {};

			float m_position = 0.f; // Position of the button within the tray

			SpritePtr m_button = nullptr;
			SpritePtr m_highlight = nullptr;
			SpritePtr m_ltray = nullptr;
			SpritePtr m_mtray = nullptr;
			SpritePtr m_rtray = nullptr;

		public:
			Slider(Rectangle destination)
			{
				m_destination = destination;
				auto ticker = Services::GetService<TimingSystem>();

				m_ltray = std::make_unique<Sprite>("Controls", Rectangle{ 384, 144, 16, 32 }, Vector4{ 1.f, 1.f, 1.f, 1.f });
				ticker->AddDrawCallback([&]() { m_ltray->Draw(); });
				m_ltray->SetDestination(Rectangle{ m_destination.x  + 8, m_destination.y + 8, 8, 16 });

				m_mtray = std::make_unique<Sprite>("Controls", Rectangle{ 400, 144, 32, 32 }, Vector4{ 1.f, 1.f, 1.f, 1.f });
				ticker->AddDrawCallback([&]() { m_mtray->Draw(); });
				m_mtray->SetDestination(Rectangle{ m_destination.x + 16, m_destination.y + 8, 112, 16 });

				m_rtray = std::make_unique<Sprite>("Controls", Rectangle{ 432, 144, 16, 32 }, Vector4{ 1.f, 1.f, 1.f, 1.f });
				ticker->AddDrawCallback([&]() { m_rtray->Draw(); });
				m_rtray->SetDestination(Rectangle{ m_destination.x + 128, m_destination.y + 8, 8, 16 });

				m_button = std::make_unique<Sprite>("Controls", Rectangle{ 320, 64, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.f }); //Vector4{ 0.09f, 0.42f, 0.78f, 1.f }
				ticker->AddDrawCallback([&]() { m_button->Draw(); });
				m_button->SetDestination(Rectangle{ m_destination.x, m_destination.y, 32, 32 });

				m_highlight = std::make_unique<Sprite>("Controls", Rectangle{ 320, 128, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.f }); //Vector4{ 0.9f, 0.9f, 0.5f, 1.f }
				ticker->AddDrawCallback([&]() { m_highlight->Draw(); });
				m_highlight->SetDestination(Rectangle{ m_destination.x, m_destination.y, 32, 32 });
			}
			~Slider()
			{

			}

			void SetDestination(DirectX::SimpleMath::Rectangle destination)
			{
				m_destination = destination;
			}

			void SetCallback(FloatCallback fn)
			{
				m_fn = fn;
			}
			void OnCallback()
			{
				m_fn(m_position);
			}

			void Update(float elapsedTime)
			{
				// Check for click in button
				// Hold for drag
				// Raise Callback with new position value
			}
		};
		using SliderPtr = std::unique_ptr<Slider>;

		/// <summary>
		/// An interactive component for scrolling window contents
		/// </summary>
		class Scroller
		{
			float  m_visible = 0.f;

		public:
			Scroller()
			{

			}
			~Scroller()
			{

			}

			void SetBufferSizes(long totalRows, long visibleRows)
			{
				m_visible = float(visibleRows / totalRows);
			}
		};
		using ScrollerPtr = std::unique_ptr<Scroller>;

		/// <summary>
		/// An interactive component for setting a binary value
		/// </summary>
		class Toggler
		{
			using BoolCallback = std::function<void(bool)>;
			BoolCallback m_fn = {};

			bool m_toggle = false;

		public:
			Toggler()
			{

			}
			~Toggler()
			{

			}

			void SetCallback(BoolCallback fn)
			{
				m_fn = fn;
			}
			void OnCallback()
			{
				m_fn(m_toggle);
			}

		};
		using TogglerPtr = std::unique_ptr<Toggler>;

		/// <summary>
		/// An element that supports displaying window title text on a bar
		/// </summary>
		class TitleBar
			: public ClayEngine::SimpleMath::Extensions::DestinationExtension
			, public Interfaces::IUIElement
		{
			SpritePtr m_sprite = nullptr;
			SpriteStringPtr m_title = nullptr;

		public:
			TitleBar()
			{
				auto ticker = Services::GetService<TimingSystem>();

				m_sprite = std::make_unique<Sprite>("Pixel", Rectangle{ 0, 0, 1, 1 }, Vector4{ 0.f, 0.f, 0.f, 1.f }, m_destination);
				ticker->AddDrawCallback([&]() { m_sprite->Draw(); });

				m_title = std::make_unique<SpriteString>(Services::GetService<ContentSystem>()->GetFont("Mason_24"));
				ticker->AddDrawCallback([&]() {m_title->Draw(); });
			}
			~TitleBar()
			{
				m_title.reset();
				m_title = nullptr;

				m_sprite.reset();
				m_sprite = nullptr;
			}

			void SetTitle(Unicode title) { m_title->SetString(title); }

			// IUIElement
			void OnLayoutChanged() override
			{
				m_sprite->SetDestination(m_destination);
				m_title->SetPosition(float((m_destination.x + (m_destination.width / 2)) - (m_title->GetWidth() / 2)), float(m_destination.y + 3));
			}
			void Update(float elapsedTime) override
			{

			}
			void Draw() override
			{

			}

		};
		using TitleBarPtr = std::unique_ptr<TitleBar>;

		/// <summary>
		/// A status bar item that represents a toggleable value
		/// </summary>
		class StatusBarTextToggler
			: public ClayEngine::SimpleMath::Extensions::DestinationExtension
			, public Interfaces::IUIElement
			// IUICallback Interface?
			// InputSystem Extension ??? This might actually also need its own callback to the Sensorium... not sure yet.
		{
			// Pixel Sprite for Text Box

			using VoidCallback = std::function<void()>; // Probably make all this part of IUICallback
			VoidCallback m_fn;

			// ToggleOnString "OVR"
			// ToggleOffString "INS"

		public:
			StatusBarTextToggler(VoidCallback fn)
			{
				m_fn = fn;
			}
			~StatusBarTextToggler()
			{

			}

			// void SetToggle() // When Update says we clicked the button

			// void OnCallback() // Tell input system to toggle insert/overwrite mode

			// Staus Bar Index for location on status bar?
			// Status Bar Affinity for left/right bound on status bar?

			// IUIElement
			void OnLayoutChanged() override
			{

			}
			void Update(float elapsedTime) override
			{

			}
			void Draw() override
			{

			}
		};
		using StatusBarTextTogglerPtr = std::unique_ptr<StatusBarTextToggler>;


		/// <summary>
		/// An element that supports displaying dynamic status data on a bar
		/// </summary>
		class StatusBar
			: public ClayEngine::SimpleMath::Extensions::DestinationExtension
			, public Interfaces::IUIElement
		{
			SpritePtr m_sprite = nullptr;

			// Left side IUIElement slots
			// Right side IUIElement slots

			// Number to track index of next element on left
			// Number to track pixel width of elements on left
			// Number to track index of next element on right
			// Number to track pixel width of elements on right

			// Number for space between buttons

			//IDEA: So, this needs to be able to update callbacks to objects that it "holds"... Is it a vector of IUICallbacks?
			// Should I create a more generic inline function to "ce::cast_to<GivenType>" similar to how ComPtr.As() works?

		public:
			StatusBar()
			{
				auto ticker = Services::GetService<TimingSystem>();

				m_sprite = std::make_unique<Sprite>("Pixel", Rectangle{ 0, 0, 1, 1 }, Vector4{ 0.f, 0.f, 0.f, 1.f }, m_destination);
				ticker->AddDrawCallback([&]() { m_sprite->Draw(); });
			}
			~StatusBar()
			{
				m_sprite.reset();
				m_sprite = nullptr;
			}

			void OnLayoutChanged() override
			{
				m_sprite->SetDestination(m_destination);
			}
			void Update(float elapsedTime) override
			{

			}
			void Draw() override
			{

			}
		};
		using StatusBarPtr = std::unique_ptr<StatusBar>;

		constexpr auto c_panel_frame_element_count = 8L;

		/// <summary>
		/// A basic UI panel that can have a custom frame, title bar, status bar, and hold other UI elements
		/// Filtering uses the following bit flags to determine how to affect a border click for resize:
		/// 0x0001 top
		/// 0x0002 right
		/// 0x0004 bottom
		/// 0x0008 left
		/// 0x0010 horizontal scale
		/// 0x0020 vertical scale
		/// </summary>
		class BasicPanel
			: public ClayEngine::SimpleMath::Extensions::DestinationExtension
			, public Interfaces::IUIElement
			, public Extensions::UIElementNestingWindowExtension
		{
			class PanelFrameElement
				: public ClayEngine::SimpleMath::Extensions::DestinationExtension
				, public Interfaces::IUIElement
				, public Extensions::UIElementMaskExtension
			{
				SpritePtr m_sprite = nullptr;

			public:
				PanelFrameElement(String texture, DirectX::SimpleMath::Rectangle source, DirectX::SimpleMath::Vector4 color, UINT mask)
				{
					auto ticker = Services::GetService<TimingSystem>();

					m_mask = mask;

					m_sprite = std::make_unique<Sprite>(texture, source, color);
					ticker->AddDrawCallback([&]() { Draw(); });
				}
				//PanelFrameElement(PanelFrameElement&&) = default;
				//PanelFrameElement& operator=(PanelFrameElement&&) = default;
				//PanelFrameElement(PanelFrameElement const&) = delete;
				//PanelFrameElement& operator=(PanelFrameElement const&) = delete;
				~PanelFrameElement()
				{
					m_sprite.reset();
					m_sprite = nullptr;
				}

				// IUIElement
				void Update(float elapsedTime) override
				{

				}

				void OnLayoutChanged() override
				{
					m_sprite->SetDestination(m_destination);
				}

				void Draw() override
				{
					m_sprite->Draw();
				}
			};
			using PanelFrameElementPtr = std::unique_ptr<PanelFrameElement>;
			using PanelFrameElements = std::array<PanelFrameElementPtr, c_panel_frame_element_count>;

			InputSystemRaw m_is = nullptr;

			PanelFrameElements m_elements = {}; // clockwise 0 = top left, 7 = left
			long m_frame_size = 6L; // (Border width)

			//ScrollerPtr m_scrollbar = nullptr;
			//bool has_v_scroll = false;

			TitleBarPtr m_titlebar = nullptr;
			bool m_show_titlebar = true;
			long m_titlebar_size = 32L;

			StatusBarPtr m_statusbar = nullptr;
			bool m_show_statusbar = true;
			long m_statusbar_size = 24L;

			//bool m_show_vscroll = false;
			//bool m_show_hscroll = false;
			//bool m_show_resizebutton = false;
			//bool m_show_closebutton = false;

			// Resizing logic
			UINT m_hit_mask = 0x00;
			bool m_is_dragging = false;
			bool m_is_resizable = true;

			Mouse::ButtonStateTracker m_lmb_tracker;
			Vector2 m_lmb_drag = {};

		public:
			BasicPanel()
			{
				m_is = Services::GetService<InputSystem>();

				m_outerbox = Rectangle{ 0, 0, 800, 600 };

				//m_elements[0] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 0, 0, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x09);
				//m_elements[1] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 64, 0, 128, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x11);
				//m_elements[2] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 192, 0, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x03);
				//m_elements[3] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 192, 64, 64, 128 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x22);
				//m_elements[4] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 192, 192, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x06);
				//m_elements[5] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 64, 192, 128, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x14);
				//m_elements[6] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 0, 192, 64, 64 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x0C);
				//m_elements[7] = std::make_unique<PanelFrameElement>("Controls", Rectangle{ 0, 64, 64, 128 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x28);

				m_elements[0] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 0, 0, 54, 55 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x09);
				m_elements[1] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 54, 0, 404, 55 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x11);
				m_elements[2] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 458, 0, 54, 55 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x03);
				m_elements[3] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 458, 55, 54, 403 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x22);
				m_elements[4] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 458, 458, 54, 54 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x06);
				m_elements[5] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 54, 458, 404, 54 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x14);
				m_elements[6] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 0, 458, 54, 54 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x0C);
				m_elements[7] = std::make_unique<PanelFrameElement>("GoldBox", Rectangle{ 0, 55, 54, 403 }, Vector4{ 1.f, 1.f, 1.f, 1.0f }, 0x28);

				OnLayoutChanged();
			}
			~BasicPanel()
			{
				std::for_each(m_elements.begin(), m_elements.end(), [&](auto& element) { element.reset(); });
				
				if (m_titlebar)
				{
					m_titlebar.reset();
					m_titlebar = nullptr;
				}

				m_is = nullptr;
			}

			void SetResizable(bool resizable) { m_is_resizable = resizable; }
			void SetFrameSize(long size) { m_frame_size = size; OnLayoutChanged(); }

			void SetTitlebarSize(long size) { m_titlebar_size = size; OnLayoutChanged(); }
			void SetShowTitleBar(bool show) { m_show_titlebar = show; OnLayoutChanged(); }

			// IUIElement
			void OnLayoutChanged() override
			{
				m_elements[0]->SetDestination(m_outerbox.x, m_outerbox.y, m_frame_size, m_frame_size); // Top Left
				m_elements[1]->SetDestination(m_outerbox.x + m_frame_size, m_outerbox.y, m_outerbox.width - (2 * m_frame_size), m_frame_size); // Top
				m_elements[2]->SetDestination(m_outerbox.x + m_outerbox.width - m_frame_size, m_outerbox.y, m_frame_size, m_frame_size); // Top Right
				m_elements[3]->SetDestination(m_outerbox.x + m_outerbox.width - m_frame_size, m_outerbox.y + m_frame_size, m_frame_size, m_outerbox.height - (2 * m_frame_size)); // Right
				m_elements[4]->SetDestination(m_outerbox.x + m_outerbox.width - m_frame_size, m_outerbox.y + m_outerbox.height - m_frame_size, m_frame_size, m_frame_size); // Bottom Right
				m_elements[5]->SetDestination(m_outerbox.x + m_frame_size, m_outerbox.y + m_outerbox.height - m_frame_size, m_outerbox.width - (2 * m_frame_size), m_frame_size); // Bottom
				m_elements[6]->SetDestination(m_outerbox.x, m_outerbox.y + m_outerbox.height - m_frame_size, m_frame_size, m_frame_size); // Bottom Left
				m_elements[7]->SetDestination(m_outerbox.x, m_outerbox.y + m_frame_size, m_frame_size, m_outerbox.height - (2 * m_frame_size)); // Left

				m_innerbox = Rectangle{ m_outerbox.x + m_frame_size, m_outerbox.y + m_frame_size, m_outerbox.width - (m_frame_size * 2), m_outerbox.height - (m_frame_size * 2) };

				//TODO: Add'l innerbox calculations for title bar, status bar, scroll bars, etc.
				if (m_show_titlebar)
				{
					if (!m_titlebar)
					{
						m_titlebar = std::make_unique<TitleBar>();
						m_titlebar->SetTitle(L"Test Title Bar Text");
					}
					m_titlebar->SetDestination(m_outerbox.x + m_frame_size, m_outerbox.y + m_frame_size, m_outerbox.width - (2 * m_frame_size), m_titlebar_size);
					m_titlebar->OnLayoutChanged();

					m_innerbox.y = m_innerbox.y + m_titlebar_size;
					m_innerbox.height = m_innerbox.height - m_titlebar_size;
				}
				else
				{
					if (m_titlebar)
					{
						m_titlebar.reset();
						m_titlebar = nullptr;
					}
				}

				if (m_show_statusbar)
				{
					if (!m_statusbar)
					{
						m_statusbar = std::make_unique<StatusBar>();
					}
					m_statusbar->SetDestination(m_outerbox.x + m_frame_size, m_outerbox.y + m_outerbox.height - m_frame_size - m_statusbar_size, m_outerbox.width - (2 * m_frame_size), m_statusbar_size);
					m_statusbar->OnLayoutChanged();

					m_innerbox.y = m_innerbox.y + m_statusbar_size;
					m_innerbox.height = m_innerbox.height - m_statusbar_size;
				}
				else
				{
					if (m_statusbar)
					{
						m_statusbar.reset();
						m_statusbar = nullptr;
					}
				}

				std::for_each(m_elements.begin(), m_elements.end(), [&](auto& element) { element->OnLayoutChanged(); });

				// Add'l OnLayoutChanged calls for title bar, status bar, scroll bars, etc.
			}

			void Update(float elapsedTime) override
			{
				auto state = m_is->GetMouseState();

				// On LMB Down
				if (state.leftButton && !m_lmb_tracker.leftButton)
				{
					m_hit_mask = 0x00;

					if (m_outerbox.Contains(DirectX::SimpleMath::Vector2{ float(state.x), float(state.y) }))
					{
						if (state.y >= m_outerbox.y && state.y <= m_outerbox.y + m_frame_size)
							m_hit_mask |= 0x01; // Top
						if (state.x >= (m_outerbox.x + m_outerbox.width - m_frame_size) && state.x <= m_outerbox.x + m_outerbox.width)
							m_hit_mask |= 0x02; // Right
						if (state.y >= (m_outerbox.y + m_outerbox.height - m_frame_size) && state.y <= m_outerbox.y + m_outerbox.height)
							m_hit_mask |= 0x04; // Bottom
						if (state.x >= m_outerbox.x && state.x <= m_outerbox.x + m_frame_size)
							m_hit_mask |= 0x08; // Left
					}
				}

				// LMB held down & dragging
				if (state.leftButton && m_lmb_tracker.leftButton)
				{
					DirectX::SimpleMath::Vector2 lmb_drag = { float(state.x - m_lmb_tracker.GetLastState().x), float(state.y - m_lmb_tracker.GetLastState().y) };

					if (0x01 & m_hit_mask) // Top is clicked
					{
						m_outerbox.y += long(lmb_drag.y);
						m_outerbox.height -= long(lmb_drag.y);
					}
					if (0x02 & m_hit_mask) // Right is clicked
					{
						m_outerbox.width += long(lmb_drag.x);
					}
					if (0x04 & m_hit_mask) // Bottom is clicked
					{
						m_outerbox.height += long(lmb_drag.y);
					}
					if (0x08 & m_hit_mask) // Left is clicked
					{
						m_outerbox.x += long(lmb_drag.x);
						m_outerbox.width -= long(lmb_drag.x);
					}
					if (!m_hit_mask) // Contains is clicked
					{
						m_outerbox.x += long(lmb_drag.x);
						m_outerbox.y += long(lmb_drag.y);
					}

					OnLayoutChanged();
				}

				// On LMB Up
				if (!state.leftButton && m_lmb_tracker.leftButton)
				{

				}

				m_lmb_tracker.Update(state);
			}
			
			void Draw() override
			{
				// Unused
			}
		};
		using BasicPanelPtr = std::unique_ptr<BasicPanel>;

		class FramesPerSecond
			: public ClayEngine::SimpleMath::Extensions::DestinationExtension
		{
			int m_last_position = 0;
			int m_position = 0; // 0tl 1t 2tr 3r 4br 5b 6bl 7l
			int m_size = 24;

			Unicode m_text = L"FPS: ";

			SpriteFontPtr m_font = nullptr;

		public:
			FramesPerSecond()
			{

			}
			~FramesPerSecond()
			{

			}
			void SetSize(int size)
			{
				m_size = size;
			}
			void SetText(Unicode text)
			{
				m_text = text;
			}
			void SetAnchorPoint(int position)
			{
				m_last_position = m_position;
				m_position = position;
			}
			void Update(float elapsedTime)
			{
				if (m_position != m_last_position)
				{
					// Update DestinationExtension for new position
					m_last_position = m_position;
				}
			}
		};
		using FramesPerSecondPtr = std::unique_ptr<FramesPerSecond>;

		/// <summary>
		/// Temporary class for prototyping various UI elements
		/// </summary>
		class Sensorium
		{
			InputSystemRaw m_is = nullptr;

			Rectangle m_screen = {};

			// Temp for testing
			//ResizableWindowPtr m_chatbox = nullptr;
			InputBarPtr m_inputbar = nullptr;
			BasicPanelPtr m_basicpanel = nullptr;

			Vector2 m_position = { 15.f, 15.f };

			//SquareChasePtr m_chase = nullptr;

		public:
			Sensorium()
			{
				auto ticker = Services::GetService<TimingSystem>();
				m_is = Services::GetService<InputSystem>();
				 
				auto window = WindowSystem::GetWindowSize();
				m_screen = Rectangle{ 0, 0, window.right - window.left, window.bottom - window.top };

				m_inputbar = std::make_unique<InputBar>();
				ticker->AddUpdateCallback([&](float elapsedTime) { m_inputbar->Update(elapsedTime); });
				m_inputbar->SetDestination(Rectangle{ long(m_position.x), long(m_position.y), 1024, 32 });

				//m_chatbox = std::make_unique<ResizableWindow>(m_position, m_size);
				//ticker->AddUpdateCallback([&](float elapsedTime) { m_chatbox->Update(elapsedTime); });

				//m_slider_r = std::make_unique<Slider>(Rectangle{ m_screen.x + 100, m_screen.y + 100, 0, 0 });
				//m_slider_g = std::make_unique<Slider>();
				//m_slider_b = std::make_unique<Slider>();

				m_basicpanel = std::make_unique<BasicPanel>();
				ticker->AddUpdateCallback([&](float elapsedTime) { m_basicpanel->Update(elapsedTime); });

				//m_chase = std::make_unique<SquareChase>();
				//ticker->AddUpdateCallback([&](float elapsedTime) { m_chase->Update(elapsedTime); });
			}
			~Sensorium()
			{
				//m_slider_b.reset();
				//m_slider_b = nullptr;
				//m_slider_g.reset();
				//m_slider_g = nullptr;
				//m_slider_r.reset();
				//m_slider_r = nullptr;

				//m_chatbox.reset();
				//m_chatbox = nullptr;

				m_inputbar.reset();
				m_inputbar = nullptr;

				m_is = nullptr;
			}

			void OnLayoutChanged()
			{

			}

			void Update(float elapsedTime)
			{

			}
		};
		using SensoriumPtr = std::unique_ptr<Sensorium>;
	}
}

/*
public interface IControl
{
	bool IsVisible { get; set; }
	bool IsReadyForDisposal { get; set; }

	void Update(GameTime gameTime);
	void Draw(GameTime gameTime);
	void Dispose();
}

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace PlanetaryEmpire.Controls
{
	public abstract class BaseControl : IControl
	{
		#region Properties
		protected SensoriumContent uiTexture;
		protected SpriteBatch spriteBatch;

		public bool IsVisible { get; set; }
		public bool IsReadyForDisposal { get; set; }
		public BaseControl Parent
		{
			get { return parent; }
			set
			{
				if (value == parent) return;
				if (parent != null) parent.Children.Remove(this);
				if (value != null && !value.Children.Contains(this)) value.Children.Add(this);
			}
		}
		public List<BaseControl> Children { get; set; }
		public Vector2 Location { get; set; }
		public Vector2 Size { get; set; }
		public TreeTraversalDirection TraversalDirection { get; set; }
		public TreeTraversalType TraversalType { get; set; }
		#endregion

		#region Derived Properties
		public Vector2 DrawLocation
		{
			get
			{
				if (this == Root)
				{
					return Location;
				}
				else
				{
					return Location + Parent.DrawLocation;
				}
			}
		}
		public Rectangle Bounds { get { return new Rectangle((int)DrawLocation.X, (int)DrawLocation.Y, (int)Size.X, (int)Size.Y); } }
		public Rectangle DrawBounds { get { return new Rectangle((int)Location.X + (int)Parent.DrawLocation.X, (int)Location.Y + (int)Parent.DrawLocation.Y, (int)Size.X, (int)Size.Y); } }
		public BaseControl Root
		{
			get
			{
				BaseControl node = this;
				while (node.Parent != null)
				{
					node = node.Parent;
				}

				return node;
			}
		}
		public int Depth
		{
			get
			{
				int depth = 0;
				BaseControl node = this;
				while (node.Parent != null)
				{
					depth++;
					node = node.Parent;
				}

				return depth;
			}
		}
		#endregion

		#region Private Fields
		private BaseControl parent;
		private int enumIndex;
		#endregion

		#region Constructor
		public BaseControl(BaseControl parent, Vector2 location, Vector2 size, SpriteBatch spriteBatch, SensoriumContent uiTexture)
		{
			this.parent = parent as BaseControl;
			if (parent != null)
			{
				parent.AddNode(this);
			}
			Children = new List<BaseControl>();

			Location = location;
			Size = size;

			this.spriteBatch = spriteBatch;
			this.uiTexture = uiTexture;

			TraversalDirection = TreeTraversalDirection.TopDown;
			TraversalType = TreeTraversalType.DepthFirst;

			IsVisible = true;

			Reset();
		}
		#endregion

		#region Add/Remove Nodes
		public void AddNode(BaseControl nodeControl)
		{
			Children.Add(nodeControl as BaseControl);
		}

		public void RemoveNode(BaseControl nodeControl)
		{
			// Set Invisible and flag to be removed on next Root update.
			// TODO: Must figure out how to remove objects from the UI without causing the Enumerator to break;
		}
		#endregion

		#region Traversal Methods
		public bool IsAncestorOf(BaseControl nodeControl)
		{
			if (Children.Contains(nodeControl)) return true;
			foreach (BaseControl child in Children)
			{
				if (nodeControl.IsAncestorOf(child)) return true;
			}
			return false;
		}

		public bool IsDescendantOf(BaseControl nodeControl)
		{
			if (Parent == null) return false;
			if (nodeControl == Parent) return true;
			return Parent.IsDescendantOf(nodeControl);
		}

		public bool DoesShareHierarchyWith(BaseControl nodeControl)
		{
			if (nodeControl == this) return true;
			if (this.IsAncestorOf(nodeControl)) return true;
			if (this.IsDescendantOf(nodeControl)) return true;
			return false;
		}

		public IEnumerator GetEnumerator()
		{
			return GetEnumerable(TraversalType, TraversalDirection).GetEnumerator();
		}

		private IEnumerable GetEnumerable(TreeTraversalType type, TreeTraversalDirection direction)
		{
			switch (type)
			{
				case TreeTraversalType.DepthFirst: return GetDepthEnumerable(direction);
				case TreeTraversalType.BreadthFirst: return GetBredthEnumerable(direction);
				default: return null;
			}
		}

		private IEnumerable GetDepthEnumerable(TreeTraversalDirection direction)
		{
			// If we're top down, return this object
			if (direction == TreeTraversalDirection.TopDown)
				yield return this;

			// Check for children and enumerate them
			foreach (BaseControl child in Children)
			{
				var e = child.GetDepthEnumerable(direction).GetEnumerator();
				while (e.MoveNext())
				{
					// If we can move through this enumeration, do so and return Current
					yield return e.Current;
				}
			}

			// If we fall out of the child enumeration, return this last object as bottom-up
			if (direction == TreeTraversalDirection.BottomUp)
				yield return this;
		}

		private IEnumerable GetBredthEnumerable(TreeTraversalDirection direction)
		{
			// If we're searching from the bottom, build a stack and push children up
			if (direction == TreeTraversalDirection.BottomUp)
			{
				var stack = new Stack<BaseControl>();
				foreach (BaseControl enumerated in GetBredthEnumerable(TreeTraversalDirection.TopDown))
				{
					stack.Push(enumerated);
				}

				while (stack.Count > 0)
				{
					yield return stack.Pop();
				}

				yield break;
			}

			// Otherwise, build a queue and fill it with children in "reading order"
			var queue = new Queue<BaseControl>();
			queue.Enqueue(this);

			while (0 < queue.Count)
			{
				BaseControl queued = queue.Dequeue();
				foreach (BaseControl enumerable in queued.Children)
				{
					queue.Enqueue(enumerable);
				}

				yield return queued;
			}
		}

		private object Current
		{
			get
			{
				return Children[enumIndex];
			}
		}

		private bool MoveNext()
		{
			return ++enumIndex < Children.Count;
		}

		private void Reset()
		{
			enumIndex = -1;
		}

		#endregion

		#region ToString()
		public override string ToString()
		{
			string s = "Depth = " + Depth.ToString() + " Children = " + Children.Count.ToString();

			if (Root == this)
			{
				s += " (Root)";
			}

			return s;
		}
		#endregion

		#region Abstract and Virtual Methods
		public virtual void Update(GameTime gameTime)
		{
			if (Depth > 1) IsVisible = Parent.IsVisible;
		}

		public abstract void Draw(GameTime gameTime);

		public virtual void Dispose()
		{
			foreach (IControl child in Children)
			{
				child.Dispose();
			}
		}
		#endregion
	}
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Input.Touch;
using Microsoft.Xna.Framework.Graphics;
using PlanetaryEmpire.Controls;

namespace PlanetaryEmpire
{
	public class InterfaceComponent : Microsoft.Xna.Framework.DrawableGameComponent
	{
		#region Declarations
		private PlanetaryEmpire planetaryEmpire;
		private SpriteBatch spriteBatch;

		// private Vector2 SelectedControlMouseLocation { get; set; }
		// private Control SelectedControlGhost { get; set; }
		// private Vector2 SelectedControlSavedLocation { get; set; }
		private List<IControl> disposalList;

		public RootFrame RootControl { get; private set; }
		public SensoriumContent SensoriumContent { get; set; }
		#endregion

		#region Constructor
		public InterfaceComponent(Game game)
			: base(game)
		{
			planetaryEmpire = game as PlanetaryEmpire;
			spriteBatch = (SpriteBatch)Game.Services.GetService(typeof(SpriteBatch));

			SensoriumContent = CreateSensoriumContentObject();

			disposalList = new List<IControl>();

			RootControl = new RootFrame(GameSettings.Data.Resolution, null, null); // TODO: This will need to update if screen resolution changes, should have event monitor for OnScreenResolutionChange()
			RootControl.TraversalType = TreeTraversalType.DepthFirst;
		}
		#endregion

		#region Update
		public override void Update(GameTime gameTime)
		{
			foreach (BaseControl control in disposalList)
			{
				control.Parent.Children.Remove(control);
			}

			disposalList.Clear();

			SetTreeEnumerationParameters();
			foreach (IControl control in RootControl.Root)
			{
				if (control.IsReadyForDisposal) { disposalList.Add(control); continue; }

				control.Update(gameTime);
			}

			// TODO: Cursor Update

			base.Update(gameTime);
		}
		#endregion

		#region Draw
		public override void Draw(GameTime gameTime)
		{
			spriteBatch.Begin();

			SetTreeEnumerationParameters();
			foreach (IControl control in RootControl.Root)
			{
				control.Draw(gameTime);
			}

			// TODO: Cursor Draw

			spriteBatch.End();


			base.Draw(gameTime);
		}
		#endregion

		#region Dispose
		protected override void Dispose(bool disposing)
		{
			GameSettings.SaveGameSettingsData();

			// Call Dispose() on all UI Objects

			base.Dispose(disposing);
		}
		#endregion

		#region Helper Methods
		private SensoriumContent CreateSensoriumContentObject()
		{
			var sensoriumContent = new SensoriumContent(planetaryEmpire.InterfaceContentCollection.UITexture, new Rectangle(0, 0, 16, 16), new Rectangle(0, 16, 12, 12), new Rectangle(0, 216, 40, 40), new Rectangle(0, 28, 16, 16), planetaryEmpire.InterfaceContentCollection.Pixel, planetaryEmpire.InterfaceContentCollection.SmallFont, planetaryEmpire.InterfaceContentCollection.MediumFont, planetaryEmpire.InterfaceContentCollection.LargeFont, new Rectangle(0, 44, 16, 16));

			return sensoriumContent;
		}

		private void SetTreeEnumerationParameters()
		{
			RootControl.TraversalDirection = TreeTraversalDirection.TopDown;
			RootControl.TraversalType = TreeTraversalType.DepthFirst;
		}
		#endregion

		#region Mouse Event Handlers
		public void OnMouseEvent(object sender, MouseEventArgs e)
		{
			var mouseLocation = e.Location;

			// MouseMove causes an already pressed Close button to go back to Default state graphically....
			if (e.MouseEventType == MouseEventType.MouseLeftDown || e.MouseEventType == MouseEventType.MouseLeftUp || e.MouseEventType == MouseEventType.MouseMove)
			{
				RootControl.TraversalDirection = TreeTraversalDirection.BottomUp;
				RootControl.TraversalType = TreeTraversalType.DepthFirst;
				foreach (var control in RootControl.Root)
				{
					if (control.GetType().IsSubclassOf(typeof(BaseInteractiveControl)))
					{
						(control as BaseInteractiveControl).OnMouseEvent(sender, e);
					}
				}
			}

			if (InterfaceHelper.SelectedControl != null)
			{
				mouseLocation = e.Location; // Used to control rubber banding when mouse moves out of bounds
				(InterfaceHelper.SelectedControl as BaseInteractiveControl).OnMouseEvent(sender, e);
			}
		}
		#endregion
	}
}

*/
