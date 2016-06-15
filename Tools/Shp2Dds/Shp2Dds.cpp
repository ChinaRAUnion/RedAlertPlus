// Shp2Dds.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Palette.h"
#include <fstream>
#include "Shp.h"
#include "Converter.h"

const wchar_t usage[] = LR"(SHP to DDS Converter
Usage:
shp2dds <source.shp> <source.pal> <dest.tani>
)";

int wmain(int argn, TCHAR* argv[])
{
	ThrowIfFailed(CoInitialize(NULL));

	SetConsoleCP(CP_WINUNICODE);
	if (argn != 4)
	{
		_putws(usage);
		return 0;
	}

	try
	{
		// 1. pal
		Palette pal;
		{
			std::ifstream f(argv[2], std::ios::in | std::ios::binary);
			if (!f.good())
				throw L"Cannot open pal file.";
			pal.Load(f);
		}
		// 2. shp
		Shp shp;
		{
			std::ifstream f(argv[1], std::ios::in | std::ios::binary);
			if (!f.good())
				throw L"Cannot open shp file.";
			shp.Load(f);
		}
		// 3. convert
		Converter converter(pal, shp);
		converter.Save(argv[3]);
		std::wcout << argv[3] << L" Successfully converted.";
	}
	catch (wchar_t* err)
	{
		std::wcerr << L"Error" << err << std::endl;
		CoUninitialize();
	}
	CoUninitialize();

	system("pause");

	return 0;
}

