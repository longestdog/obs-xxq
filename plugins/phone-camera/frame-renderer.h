#pragma once

#include <d3d9.h>
#include <graphics/graphics.h>
extern "C" {
#include "libavformat/avformat.h"
}

class D3D9Renderer {
public:
	D3D9Renderer();
	~D3D9Renderer();

	bool init();
	void destroy();

	IDirect3DDevice9 *getDevice();
	void renderFrame(AVFrame *frame);
	gs_texture_t *outputTexture() { return m_outputTexture; }

protected:
	bool createDevice();
	bool createTexture(int width, int height);

	HWND m_hwnd = NULL;

	IDirect3D9Ex *m_d3d9 = NULL;
	IDirect3DDevice9Ex *m_d3d9Device = NULL;

	int m_width = 0;
	int m_height = 0;
	IDirect3DTexture9 *m_outputD3D9Texture = NULL;
	IDirect3DSurface9 *m_renderSurface = NULL;
	HANDLE m_sharedHandler = NULL;

	gs_texture_t *m_outputTexture = nullptr;
};
