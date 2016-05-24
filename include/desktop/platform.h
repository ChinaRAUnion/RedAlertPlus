//
// Tomato Media
// ƽ̨��� for Windows Desktop
// ���ߣ�SunnyCase
// ����ʱ�䣺2015-08-05
//
#pragma once
#include <comdef.h>
#include <wrl.h>
#include <stdexcept>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		_com_raise_error(hr, nullptr);
#else
		_com_raise_error(hr);
#endif
}

struct tomato_error
{
	HRESULT hr;
	const wchar_t* message;
};

inline void ThrowIfFailed(HRESULT hr, const wchar_t* message)
{
	if (FAILED(hr))
		throw tomato_error{ hr,message };
}

template<typename T>
void ThrowWin32IfNot(T value)
{
	if (!value)
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
}

template<typename T, typename = std::enable_if_t<std::is_scalar<T>::value>>
void ThrowIfNot(T value, const wchar_t* message)
{
	if (!value)
		ThrowIfFailed(E_FAIL, message);
}

template<typename T, typename = std::enable_if_t<!std::is_scalar<T>::value>>
void ThrowIfNot(const T& value, const wchar_t* message)
{
	if (!value)
		ThrowIfFailed(E_FAIL, message);
}

inline void __declspec(noreturn) ThrowAlways(const wchar_t* message)
{
	ThrowIfFailed(E_FAIL, message);
}

#define CATCH_ALL() catch(tomato_error& ex){ return ex.hr;}catch(_com_error& ex){return ex.Error();}catch(...){return E_FAIL;}
#define CATCH_ALL_WITHHR(hr) catch(tomato_error& ex){ hr = ex.hr; }catch(_com_error& ex){ hr = ex.Error(); }catch(...){ hr = E_FAIL; }
#define CATCH_ALL_WITHEVENT(event) catch(tomato_error& ex){ event.set_exception(ex); }catch(_com_error& ex){event.set_exception(ex);}catch(...){event.set_exception(E_FAIL);}

namespace WRL = Microsoft::WRL;

// ��ģ����
extern HMODULE ModuleHandle;

#include <string>
#include <winstring.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
inline std::wstring HStringToWString(HSTRING hStr)
{
	UINT32 length;
	auto str = WindowsGetStringRawBuffer(hStr, &length);
	return{ str, length };
}
#endif