#pragma once
#include "DeviceContext.h"

DEFINE_NS_ENGINE

// �����򶥵���ɫ������ MVP ����ĳ�����������
struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

END_NS_ENGINE