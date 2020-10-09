#include <tchar.h>

#include "Application.h"
#include "D3D11.h"

D3D11 g_d3d11;

Application::Application () : m_hInstance (nullptr), m_hWnd (nullptr)
{
}

Application::~Application ()
{
}

bool Application::Initialize (LPCTSTR name, unsigned int width, unsigned int height, bool isFullScreen)
{
	// ������ Ŭ���� �̸� �����Ѵ�.
	size_t classNameLength = _tcslen (name);
	m_className = std::make_unique<TCHAR[]> (classNameLength + 1);
	_tcsnccpy_s (m_className.get (), classNameLength + 1, name, classNameLength);

	// exe ���� �ڵ��� �����´�.
	m_hInstance = GetModuleHandle (nullptr);

	// ������ Ŭ������ �����Ѵ�.
	WNDCLASS wc = { };
	wc.hInstance = m_hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = m_className.get ();
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor (nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon (nullptr, IDI_WINLOGO);

	// ������ Ŭ������ ����Ѵ�.
	if (RegisterClass (&wc) == false)
	{
		return false;
	}

	// ��üȭ������ �ƴ����� ���� ��Ÿ���� �����Ѵ�.
	DWORD style = 0;

	if (isFullScreen)
	{
		width = GetSystemMetrics (SM_CXSCREEN);
		height = GetSystemMetrics (SM_CYSCREEN);
		style |= WS_POPUP;
	}
	else
	{
		style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}

	// ȭ�� ũ�⸦ �����´�.
	UINT screenWidth = GetSystemMetrics (SM_CXSCREEN);
	UINT screenHeight = GetSystemMetrics (SM_CYSCREEN);

	// ȭ�� ũ��� ������ ũ�⸦ ��������
	// �����츦 ȭ�� �߾ӿ� ���ߴ� Rect�� ����Ѵ�.
	RECT windowRect = { };
	windowRect.left = (screenWidth / 2) - (width / 2);
	windowRect.top = (screenHeight / 2) - (height / 2);
	windowRect.right = (screenWidth / 2) + (width / 2);
	windowRect.bottom = (screenHeight / 2) + (height / 2);

	// ����� �簢���� Client Area�� ��
	// ������ ��Ÿ�Ͽ� �´� ������ Rect�� ����Ѵ�.
	AdjustWindowRect (&windowRect, style, false);

	UINT windowX = windowRect.left;
	UINT windowY = windowRect.top;
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;

	// �����츦 �����Ѵ�.
	m_hWnd = CreateWindow (
		m_className.get (),
		m_className.get (),
		style,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		m_hInstance,
		nullptr
	);

	if (m_hWnd == nullptr)
	{
		return false;
	}

	// �����츦 ǥ���Ѵ�.
	ShowWindow (m_hWnd, SW_SHOW);

	// D3D11�� �ʱ�ȭ�Ѵ�.
	if (g_d3d11.Initialize (m_hWnd, width, height, false, true, 60, true, 4) == false)
	{
		return false;
	}

	return true;
}

void Application::Shutdown ()
{
	// D3D11�� �����Ѵ�.
	g_d3d11.Shutdown ();

	// ������ �����츦 �ı��Ѵ�.
	DestroyWindow (m_hWnd);
	m_hWnd = nullptr;

	// ����� ������ Ŭ������ �����Ѵ�.
	UnregisterClass (m_className.get (), m_hInstance);
	m_hInstance = nullptr;
}

void Application::Update ()
{
	MSG message = { };

	while (true)
	{
		// �޽����� ������ ó���Ѵ�.
		while (PeekMessage (&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				return;
			}

			TranslateMessage (&message);
			DispatchMessage (&message);
		}

		g_d3d11.ClearSwapChain (0.0f, 0.0f, 0.0f, 1.0f);
		g_d3d11.PresentSwapChain (true);
	}
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage (0);
			return 0;
		}
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}
