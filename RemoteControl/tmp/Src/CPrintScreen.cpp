#pragma once
#include <CPrintScreen.h>

CPrintScreen::CPrintScreen()
{
    m_hInstance = GetModuleHandle(NULL);
    m_nWidth = 800;
	m_nHeight = 600;

    m_captureScreen = nullptr;
}

CPrintScreen::CPrintScreen(HINSTANCE hInstance)
{
	this->m_hInstance = hInstance;
	m_nWidth = 800;
	m_nHeight = 600;

	m_captureScreen = nullptr;
}

CPrintScreen::~CPrintScreen()
{
}