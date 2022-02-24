#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine WindowSystem API Class (C) 2022 Epoch Meridian, LLC.            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "Strings.h"
#include "Services.h"

namespace ClayEngine
{
	namespace Platform
	{
		constexpr auto c_min_window_width = 320;
		constexpr auto c_min_window_height = 200;

		/// <summary>
		/// The window class maintains the various system handles for the Win32 window
		/// </summary>
		class WindowSystem
		{
			static inline HINSTANCE m_instance_handle;
			static inline HWND m_window_handle;

			static inline Unicode m_class_name = { L"ClayEngineWindowClass" };
			static inline Unicode m_window_name = { L"ClayEngine Client Prototype - Milestone 0.0.2" };

			static inline RECT m_size;

			static inline Functions s_onwindowsizechanged = {};
			static inline Functions s_onactivated = {};
			static inline Functions s_ondeactivated = {};
			static inline Functions s_onsuspended = {};
			static inline Functions s_onresumed = {};

			static ATOM MakeWindowClass(WNDPROC fn);
			static BOOL MakeWindowInstance(int flags);

		public:
			static void Initialize(HINSTANCE instance, int flags, WNDPROC fn, RECT size);
			static void SetWindowSize(RECT size);
			static void UpdateWindowSize();
			static void SetWindowTitle(Unicode title);
			static const HWND GetWindowHandle();
			static const RECT& GetWindowSize();
			static const int GetWindowWidth();
			static const int GetWindowHeight();


			static void AddOnWindowSizeChangedCallback(Function fn);
			static void OnWindowSizeChanged();

			static void AddOnActivatedCallback(Function fn);
			static void OnActivated();

			static void AddOnDeactivatedCallback(Function fn);
			static void OnDeactivated();

			static void AddOnSuspendingCallback(Function fn);
			static void OnSuspending();

			static void AddOnResumingCallback(Function fn);
			static void OnResuming();
		};
		using WindowSystemPtr = std::unique_ptr<WindowSystem>;
		using WindowSystemRaw = WindowSystem*;
	}
}
