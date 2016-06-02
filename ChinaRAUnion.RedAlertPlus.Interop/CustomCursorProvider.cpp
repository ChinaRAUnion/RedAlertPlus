//
// Red Alert Plus
// CustomCursorProvider
// 作者：SunnyCase
// 创建时间：2016-06-01
//
#include "pch.h"
#include "CustomCursorProvider.h"
#include "../ChinaRAUnion.RedAlertPlus.Assets/resource.h"

using namespace NS_RAP;
using namespace WRL;
using namespace Windows::UI::Core;

Windows::UI::Core::CoreCursor ^ CustomCursorProvider::GetCursor(CustomCursors cursor)
{
	static std::map<int, int> ids =
	{
		{ int(CustomCursors::NoDrop), IDC_NODROP }
	};

	auto it = ids.find(int(cursor));
	if (it != ids.end())
		return ref new CoreCursor(CoreCursorType::Custom, it->second);
	ThrowIfFailed(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), L"No such cursor found.");
}
