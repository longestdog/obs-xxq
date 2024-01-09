#include "frame-renderer.h"
#include <obs.h>
#include <util/base.h>

#define DX_SAFE_RELEASE(p)              \
	{                               \
		if (p) {                \
			(p)->Release(); \
			(p) = NULL;     \
		}                       \
	}

D3D9Renderer::D3D9Renderer() {}

D3D9Renderer::~D3D9Renderer()
{
	destroy();
}

bool D3D9Renderer::init()
{
	m_hwnd = GetDesktopWindow();

	if (!createDevice()) {
		return false;
	}

	return true;
}

void D3D9Renderer::destroy()
{
	DX_SAFE_RELEASE(m_renderSurface);
	DX_SAFE_RELEASE(m_outputD3D9Texture);
	DX_SAFE_RELEASE(m_d3d9Device);
	DX_SAFE_RELEASE(m_d3d9);

	obs_enter_graphics();
	gs_texture_destroy(m_outputTexture);
	obs_leave_graphics();
}

IDirect3DDevice9 *D3D9Renderer::getDevice()
{
	return m_d3d9Device;
}

bool D3D9Renderer::createDevice()
{
	D3DCAPS9 d3d9_caps_;
	D3DPRESENT_PARAMETERS present_params_;
	memset(&d3d9_caps_, 0, sizeof(D3DCAPS9));
	memset(&present_params_, 0, sizeof(D3DPRESENT_PARAMETERS));

	HRESULT hr = S_OK;

	hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_d3d9);
	if (!m_d3d9) {
		blog(LOG_DEBUG, "Direct3DCreate9() failed.");
		return false;
	}

	hr = m_d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3d9_caps_);
	if (FAILED(hr)) {
		blog(LOG_DEBUG, "IDirect3D9::GetDeviceCaps() failed, %x", hr);
		return false;
	}

	int behavior_flags = D3DCREATE_MULTITHREADED;
	if (d3d9_caps_.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		behavior_flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	} else {
		behavior_flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	D3DDISPLAYMODE disp_mode;
	m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &disp_mode);

	present_params_.BackBufferWidth = 640;
	present_params_.BackBufferHeight = 480;
	present_params_.BackBufferFormat = disp_mode.Format; //D3DFMT_X8R8G8B8;
	present_params_.BackBufferCount = 1;
	present_params_.MultiSampleType = D3DMULTISAMPLE_NONE;
	present_params_.MultiSampleQuality = 0;
	present_params_.SwapEffect = D3DSWAPEFFECT_COPY;
	present_params_.hDeviceWindow = m_hwnd;
	present_params_.Windowed = TRUE;
	present_params_.Flags = D3DPRESENTFLAG_VIDEO;
	present_params_.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	present_params_.PresentationInterval = 0; // D3DPRESENT_INTERVAL_IMMEDIATE;

	hr = m_d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, // primary adapter
				    D3DDEVTYPE_HAL,     // device type
				    m_hwnd,             // window associated with device
				    behavior_flags,     // vertex processing
				    &present_params_,   // present parameters
				    NULL,
				    &m_d3d9Device // return created device
	);

	return true;
}

bool D3D9Renderer::createTexture(int width, int height)
{
	if (!m_d3d9Device) {
		return false;
	}

	DX_SAFE_RELEASE(m_outputD3D9Texture);
	DX_SAFE_RELEASE(m_renderSurface);
	m_sharedHandler = NULL;

	HRESULT hr = S_OK;

	hr = m_d3d9Device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_outputD3D9Texture, &m_sharedHandler);

	if (FAILED(hr)) {
		blog(LOG_DEBUG, "IDirect3DDevice9::CreateTexture() failed, %x", hr);
		return false;
	}

	hr = m_outputD3D9Texture->GetSurfaceLevel(0, &m_renderSurface);
	if (FAILED(hr)) {
		blog(LOG_DEBUG, "IDirect3DTexture9::GetSurfaceLevel() failed, %x", hr);
		DX_SAFE_RELEASE(m_outputD3D9Texture);

		return false;
	}

	m_width = width;
	m_height = height;
	return true;
}

void D3D9Renderer::renderFrame(AVFrame *frame)
{
	if (!m_d3d9Device) {
		return;
	}

	LPDIRECT3DSURFACE9 surface = (LPDIRECT3DSURFACE9)frame->data[3];

	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);

	if (m_width != frame->width || m_height != frame->height) {
		if (!createTexture(frame->width, frame->height)) {
			return;
		}
	}

	RECT src_rect = {0, 0, frame->width, frame->height};
	m_d3d9Device->StretchRect(surface, &src_rect, m_renderSurface, NULL, D3DTEXF_LINEAR);

	if (!m_outputTexture) {
		m_outputTexture = gs_texture_open_shared((uint32_t)m_sharedHandler);
	}
}
