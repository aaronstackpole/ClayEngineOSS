#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Strings Library (C) 2022 Epoch Meridian, LLC.                   */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <mutex>

namespace ClayEngine
{

	using Unicode = std::wstring;
	using UnicodeStrings = std::vector<std::wstring>;
    using String = std::string;
	using Strings = std::vector<std::string>;
	
	/// <summary>
	/// Convert an ANSI string to a Unicode string
	/// </summary>
	inline Unicode ToUnicode(String string)
    {
        std::wstringstream ss;
        ss << string.c_str();
        return ss.str().c_str();
    }

	/// <summary>
    /// Write a Unicode string to the console
    /// </summary>
    inline void WriteLine(Unicode message)
    {
        std::wcout << L"[" << std::setfill(L'0') << std::setw(8) << std::this_thread::get_id() << L"] " << message << std::endl;
    }

    /// <summary>
    /// Convert a Unicode string to an ANSI string
    /// </summary>
    inline String ToString(Unicode string)
    {
        std::stringstream ss;
        ss << string.c_str();
        return ss.str().c_str();
    }

	/// <summary>
	/// Write an ANSI string to the console
	/// </summary>
	/// <param name="message"></param>
	inline void WriteLine(String message)
    {
		//TODO: This should just convert ToUnicode and use Unicode WriteLine, or the other way... pick one.
        std::cout << "[" << std::setfill('0') << std::setw(8) << std::this_thread::get_id() << "] " << message << std::endl;
    }

	/// <summary>
	/// Returns a vector of ANSI strings based on an input string and a char to delimit the tokens
	/// </summary>
	inline Strings LineSplit(const String& line, const char delimiter)
	{
		std::istringstream iss(line); // Turn the line into an stringstream so we can getline on it

		Strings v = {};
		String s;

		while (std::getline(iss, s, delimiter))
		{
			v.push_back(s);
		}

		return v;
	}

	class LexicalAnalyzer
	{
		String m_string;
		String::const_iterator m_iter;

	public:
		LexicalAnalyzer(String input) : m_string{ input }
		{
			m_iter = m_string.cbegin();
		}
		~LexicalAnalyzer() = default;

		String GetNextToken()
		{
			//TODO: See SplitLine for more samples...
			return "";
		}

		bool IsNumber(char ch)
		{
			return (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' ||
				ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9');
		}

		bool IsOperator(char ch)
		{
			return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '>' ||
				ch == '<' || ch == '=' || ch == '|' || ch == '&' || ch == '{' ||
				ch == '}' || ch == '[' || ch == ']' || ch == '(' || ch == ')' ||
				ch == '\'' || ch == '\"' || ch == '`' || ch == '.' || ch == ',');
		}

		bool IsWhitespace(char ch)
		{
			return (ch == ' ' || ch == '\t' || ch == '\n');
		}
	};
}