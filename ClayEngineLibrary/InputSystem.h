#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine InputSystem API Class Library (C) 2022 Epoch Meridian, LLC.     */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "ClayEngine.h"
#include "Mouse.h"

namespace ClayEngine
{
	template<size_t Size>
	class StringBuffer
	{
		static_assert(Size >= 0);
		static_assert(Size <= c_max_stringbuffer_length);

		using Buffer = std::array<wchar_t, Size>;
		Buffer m_buffer = {};
		std::mutex m_buffer_mtx = {};

		// Internally, we want to store this as a signed long int to support negative index movement
		int64_t m_size = static_cast<int64_t>(Size);

		int64_t m_carat = 0;
		int64_t m_end = 0;

		bool m_is_overwrite = false;

	public:
		StringBuffer() = default;
		~StringBuffer() = default;

		/// <summary>
		/// Return to the caller where in the buffer the carat is currently located (use for rendering cursor, for example)
		/// </summary>
		size_t GetCarat()
		{
			m_buffer_mtx.lock();
			auto r = static_cast<size_t>(m_carat);
			m_buffer_mtx.unlock();
			return r;
		}

		/// <summary>
		/// Move the carat forward or backwards in the buffer, bounds are checked against size constant
		/// </summary>
		void MoveCarat(const int64_t offset)
		{
			if (m_carat == 0 && m_end == 0)
			{
				WriteLine("StringBuffer MoveCarat DEBUG: Cannot move carat on empty string.");
				return;
			}

			auto v = m_carat + offset;

			m_buffer_mtx.lock();
			if (v >= m_end)
			{
				m_carat = m_end;
			}
			else if (v >= m_size)
			{
				m_carat = m_size;
			}
			else if (v < 0)
			{
				m_carat = 0;
			}
			else
			{
				m_carat = v;
			}
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// Return if the buffer is in insert or overwrite mode, this controls the behavior of how we parse OnChar messages.
		/// </summary>
		bool GetOverwrite()
		{
			m_buffer_mtx.lock();
			auto v = m_is_overwrite;
			m_buffer_mtx.unlock();
			
			return v;
		}

		/// <summary>
		/// Set the buffer insert/overwrite mode to the mode that is provided.
		/// </summary>
		/// <param name="value">true = overwrite, false = insert</param>
		void SetOverwrite(bool overwrite)
		{
			m_buffer_mtx.lock();
			m_is_overwrite = overwrite;
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// Toggle whatever the current insert/overwrite mode is to the other mode
		/// </summary>
		void ToggleOverwrite()
		{
			m_buffer_mtx.lock();
			m_is_overwrite = !m_is_overwrite;
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// A function to insert a character into the buffer, will push chars forward or append
		/// </summary>
		void InsertChar(const wchar_t character)
		{
			m_buffer_mtx.lock();
			if (m_carat >= m_size) //TODO: What to do with full buffer?
			{
				WriteLine("StringBuffer InsertChar DEBUG: We don'm_thread know what to do with a full buffer yet, doing nothing.");
				m_buffer_mtx.unlock();
				return;
			}

			if (m_carat == m_end) // Normal condition
			{
				m_buffer[m_carat] = character;

				++m_carat;
				++m_end;
			}
			if (m_carat < m_end)
			{
				if (m_is_overwrite)
				{
					// Replace character in array with character provided
					m_buffer[m_carat] = character;

					++m_carat;
				}
				else
				{
					// Push values between carat and end forward one... Check 1024 bounds
					auto v = m_end + 1;

					if (v >= m_size)
					{
						m_buffer_mtx.unlock();
						return;
					}

					for (; v > m_carat; --v)
					{
						m_buffer[v] = m_buffer[v - 1];
					}
					m_buffer[m_carat] = character;

					++m_carat;
					++m_end;
				}
			}
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// Function will delete the character before the cursor in the buffer and move the carat back
		/// </summary>
		void RemovePrev()
		{
			m_buffer_mtx.lock();
			if (m_carat == 0)
			{
				WriteLine("StringArray::RemovePrev DEBUG: We're at the beginning of the line, can'm_thread backspace.");
				m_buffer_mtx.unlock();
				return;
			}

			if (m_carat == m_end)
			{
				--m_carat;

				m_buffer[m_carat] = 0;

				--m_end;
			}
			else
			{
				--m_carat;
				auto v = m_carat;

				for (; v < m_end; ++v)
				{
					m_buffer[v] = m_buffer[v + 1ull];
				}
				m_buffer[v] = 0;

				--m_end;
			}
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// Function will delete the character after the cursor and shuffle the buffer back
		/// </summary>
		void RemoveNext()
		{
			m_buffer_mtx.lock();
			if (m_end == 0)
			{
				WriteLine("StringArray::RemoveNext DEBUG: There's no text to delete.");
				m_buffer_mtx.unlock();
				return;
			}

			if (m_end >= m_size)
			{
				WriteLine("StringArray::RemoveNext DEBUG: We're at the end of the line, can'm_thread delete.");
				m_buffer_mtx.unlock();
				return;
			}

			auto v = m_carat;

			for (; v < m_end; ++v)
			{
				m_buffer[v] = m_buffer[v + 1ull];
			}
			m_buffer[v] = 0;

			--m_end;
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// Returns the buffer as a Unicode (std::wstring) string
		/// </summary>
		Unicode GetString()
		{
			m_buffer_mtx.lock();
			auto r = Unicode{ m_buffer.data() };
			m_buffer_mtx.unlock();

			return r;
		}

		/// <summary>
		/// Replace the buffer with the provided Unicode string. This function does dangerous low-level memory operations
		/// while inside the mutex, be careful of locks.
		/// </summary>
		void SetString(const Unicode& string)
		{
			m_buffer_mtx.lock();
			m_buffer.fill(0);
			m_carat = 0;
			m_end = 0;

			ThrowIfFailed(memcpy_s(&m_buffer, sizeof(wchar_t) * string.length(), string.c_str(), sizeof(wchar_t) * string.length()));

			m_carat = static_cast<int64_t>(string.length());
			m_end = static_cast<int64_t>(string.length());
			m_buffer_mtx.unlock();
		}

		/// <summary>
		/// Clear the buffer, typically used when you pull the string out and stick it somewhere else so you can start a new line
		/// </summary>
		void Clear()
		{
			m_buffer_mtx.lock();
			m_buffer.fill(0);

			m_carat = 0;
			m_end = 0;
			m_buffer_mtx.unlock();
		}
	};

	template<size_t Size>
	class StringArray
	{
		static_assert(Size >= 0);
		static_assert(Size <= c_max_stringarray_length);

		using Buffer = std::array<Unicode, Size>;
		Buffer m_buffer = {};
		std::mutex m_buffer_mtx = {};

		int64_t m_size = static_cast<int64_t>(Size);

		int64_t m_carat = 0;
		int64_t m_end = 0;

	public:
		StringArray() = default;
		~StringArray() = default;

		/// <summary>
		/// Rudimentary insert into a vector of Unicode strings
		/// </summary>
		void InsertString(const Unicode& string)
		{
			if (m_carat < m_end) { m_carat = m_end; }

			if (m_carat < m_size)
			{
				m_buffer[m_carat] = string;

				++m_carat;
				++m_end;
			}
			else
			{
				for (int64_t i = 1; i < m_size; ++i)
				{
					m_buffer[i - 1] = m_buffer[i];
				}

				m_buffer[m_carat - 1] = string;
			}
		}

		/// <summary>
		/// Return the string at the scrollback carat location. Copy to StringBuffer to work on it. Prefer to use
		/// InsertString to add a new copy of the edited line into the end of the buffer instead of UpdateString.
		/// </summary>
		Unicode GetString()
		{
			auto r = Unicode{ L"" };
			if (m_carat == m_end) return r;

			m_buffer_mtx.lock();
			r = m_buffer[m_carat];
			m_buffer_mtx.unlock();
			
			return r;
		}

		size_t GetCarat()
		{
			m_buffer_mtx.lock();
			auto r = static_cast<size_t>(m_carat);
			m_buffer_mtx.unlock();
			
			return r;
		}

		void MoveCarat(int64_t offset)
		{
			if (m_carat == 0 && m_end == 0)
			{
				WriteLine("StringArray MoveCarat DEBUG: Cannot move carat on empty buffer.");
				return;
			}

			auto v = m_carat + offset;

			m_buffer_mtx.lock();
			if (v >= m_end)
			{
				m_carat = m_end;
			}
			else if (v >= m_size)
			{
				m_carat = m_size;
			}
			else if (v < 0)
			{
				m_carat = 0;
			}
			else
			{
				m_carat = v;
			}
			m_buffer_mtx.unlock();
		}

		bool IsInScrollback()
		{
			return m_carat < m_end;
		}
		
		size_t GetBackBufferSize()
		{
			return static_cast<size_t>(m_end);
		}

		void Clear()
		{
			m_buffer_mtx.lock();
			m_buffer.fill(0);

			m_carat = 0;
			m_end = 0;
			m_buffer_mtx.unlock();
		}
	};

    namespace Platform
    {
        using MousePtr = std::unique_ptr<DirectX::Mouse>;
        using Tracker = DirectX::Mouse::ButtonStateTracker;

        using InputBuffer = StringBuffer<c_max_stringbuffer_length>;
        using InputBufferPtr = std::unique_ptr<InputBuffer>;
        using InputBufferRaw = InputBuffer*;

        using ScrollbackBuffer = StringArray<c_max_scrollback_length>;
        using ScrollbackBufferPtr = std::unique_ptr<ScrollbackBuffer>;
        using ScrollbackBufferRaw = ScrollbackBuffer*;

		using DisplayBuffer = StringArray<c_max_displaybuffer_length>;
		using DisplayBufferPtr = std::unique_ptr<DisplayBuffer>;
		using DisplayBufferRaw = DisplayBuffer*;

        /// <summary>
        /// Input system is the top level API for handling user input events
        /// </summary>
        class InputSystem
        {
            //IDEA: It might make sense to have this class have a state related to input mode
            // Or we could have this class collect input into a buffer regardless and just not use it
            // when it's not needed? Would that be wasting resources tracking key presses as a string
            // when we're just in move mode and most people are using WASD?

            MousePtr m_mouse = nullptr;
            Tracker m_tracker = {};

            InputBufferPtr m_input_buffer = nullptr;
            ScrollbackBufferPtr m_scrollback_buffer = nullptr;
			DisplayBufferPtr m_display_buffer = nullptr;

            bool m_caps_lock = false;
            bool m_shift_pressed = false;
            bool m_control_pressed = false;
            bool m_alt_pressed = false;

        public:
            InputSystem();
            ~InputSystem();

            void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam);
            DirectX::Mouse::State GetMouseState();
            DirectX::Mouse::ButtonStateTracker& GetButtonStateTracker();

            void OnKeyDown(WPARAM wParam, LPARAM lParam);
            void OnKeyUp(WPARAM wParam, LPARAM lParam);
            void OnChar(WPARAM wParam, LPARAM lParam);

            Unicode GetBuffer();

            size_t GetCaratIdx() { return m_input_buffer->GetCarat(); }
            bool GetOverwrite() { return m_input_buffer->GetOverwrite(); }
            ScrollbackBufferRaw GetBackBuffer() { return m_scrollback_buffer.get(); }
            size_t GetBackBufferSize() { return m_scrollback_buffer->GetBackBufferSize(); }

            void AddMessage(Unicode string) { m_scrollback_buffer->InsertString(string); }

            //TODO: There needs to be an accessor to insert a string into the buffer so messages from others will show up
        };
        using InputSystemPtr = std::unique_ptr<InputSystem>;
        using InputSystemRaw = InputSystem*;
    }
}