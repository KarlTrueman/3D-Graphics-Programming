#pragma once

// Define our own version of basic_stringbuf to redirect cout to so we can
// output to the Visual Studio output pane rather than to a console
template<typename TChar, typename TTraits = std::char_traits<TChar>>
class OutputDebugStringBuf : public std::basic_stringbuf<TChar, TTraits> {
public:
	typedef std::basic_stringbuf<TChar, TTraits> BaseClass;

	explicit OutputDebugStringBuf() : _buffer(256) {
		BaseClass::setg(nullptr, nullptr, nullptr);
		BaseClass::setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
	}

	static_assert(std::is_same<TChar, char>::value || std::is_same<TChar, wchar_t>::value, "OutputDebugStringBuf only supports char and wchar_t types");

	int sync() override try {
		MessageOutputer<TChar, TTraits>()(BaseClass::pbase(), BaseClass::pptr());
		BaseClass::setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
		return 0;
	}
	catch (...) {
		return -1;
	}

	typename BaseClass::int_type overflow(typename BaseClass::int_type c = TTraits::eof()) override {
		auto syncRet = sync();
		if (c != TTraits::eof()) {
			_buffer[0] = c;
			BaseClass::setp(_buffer.data(), _buffer.data() + 1, _buffer.data() + _buffer.size());
		}
		return syncRet == -1 ? TTraits::eof() : 0;
	}
private:
	std::vector<TChar> _buffer;

	template<typename TChar, typename TTraits>
	struct MessageOutputer;

	template<>
	struct MessageOutputer<char, std::char_traits<char>> {
		template<typename TIterator>
		void operator()(TIterator begin, TIterator end) const {
			std::string s(begin, end);
			OutputDebugStringA(s.c_str());
		}
	};

	template<>
	struct MessageOutputer<wchar_t, std::char_traits<wchar_t>> {
		template<typename TIterator>
		void operator()(TIterator begin, TIterator end) const {
			std::wstring s(begin, end);
			OutputDebugStringW(s.c_str());
		}
	};
};

// If you get multiple definitions related to this function you are including this header more
// than once which you should not be doing. It is just included from main.cpp
void RedirectStandardOuput()
{
	static OutputDebugStringBuf<char> charDebugOutput;
	std::cout.rdbuf(&charDebugOutput);
	std::cerr.rdbuf(&charDebugOutput);
	std::clog.rdbuf(&charDebugOutput);

	static OutputDebugStringBuf<wchar_t> wcharDebugOutput;
	std::wcout.rdbuf(&wcharDebugOutput);
	std::wcerr.rdbuf(&wcharDebugOutput);
	std::wclog.rdbuf(&wcharDebugOutput);
}