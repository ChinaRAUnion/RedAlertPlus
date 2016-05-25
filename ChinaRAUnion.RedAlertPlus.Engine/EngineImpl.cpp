//
// Red Alert Plus Engine
// Engine ʵ��
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-25
//
#include "pch.h"
#include "EngineImpl.h"
#include <sstream>

HRESULT STDMETHODCALLTYPE CreateEngine(NS_ENGINE::IEngine** engine, NS_ENGINE::IResourceResovler* resourceResolver)
{
	try
	{
		ARGUMENT_NOTNULL_HR(engine);
		*engine = WRL::Make<NS_ENGINE::Engine>(resourceResolver).Detach();
		return S_OK;
	}
	CATCH_ALL();
}

using namespace NS_ENGINE;
using namespace WRL;

Engine::Engine(IResourceResovler* resourceResolver)
	:_resourceResolver(resourceResolver)
{

}

void Engine::SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler)
{
	_deviceContext.SetSwapChainChangedHandler(std::move(handler));
}

void Engine::UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi)
{
	_deviceContext.UpdateDisplayMetrics(logicalWidth, logicalHeight, rotation, compositionScaleX, compositionScaleY, dpi);
}
