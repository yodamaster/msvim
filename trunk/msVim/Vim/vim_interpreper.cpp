#include "stdafx.h"
#include <queue>
#include "vim.h"

/* construct a one-character BSTR */
static BYTE s_BSTR_1Charactor_string[10];
static BSTR s_ch;
static void s_ClearChar();
static void s_SetChar(DWORD ch);

/* handle unicode character */
static std::queue<BYTE> s_isUniChar;
static BYTE s_lobyte;
static BYTE s_hibyte;

/* input characters in the window */
static void s_TypeChar(PVIMProp vProp);

/* input command characters in the window */
static HWND s_wnd_working_child;
static void s_TypeCmdChar(PVIMProp vProp);

int VimInterpreter(HWND hVim, UINT msg, WPARAM wParam, LPARAM lParam, PVIMProp vimProp) {
	switch (msg) {
	case WM_CHAR:
		{
			if (s_isUniChar.empty()) {
				s_SetChar(wParam);
			}
			else {
				BYTE& rate = s_isUniChar.front();
				if (rate == 2) {
					--rate;
					s_lobyte = wParam;
					break;
				}
				else if (rate == 1) {
					s_isUniChar.pop();
					s_hibyte = wParam;
					DWORD wideChar = MAKEWORD(s_lobyte, s_hibyte);
					s_ClearChar();
					int nUTF16Size = ::MultiByteToWideChar(
						CP_ACP,
						0,
						(LPCSTR)&wideChar,
						4,
						s_ch,
						3);
					if (nUTF16Size == 0) {
						return -1;
					}
				}
				else {
					s_isUniChar.pop();
					s_SetChar(wParam);
				}
			}
			
			if ( (HIBYTE(HIWORD(lParam)) & 0x0001) == 1 ) {
				// ¿ì½Ý¼ü
				return -1;
			}
			else {
				// ASCII×Ö·û
				if (vimProp->input_mode == vm_insert) {
					s_TypeChar(vimProp);
				}
				else if (vimProp->input_mode == vm_command) {
					s_TypeCmdChar(vimProp);
				}
			}
		}
		break;

	case WM_IME_CHAR:
		{
			if (wParam > 0x7f) {
				s_isUniChar.push(2);
			}
			else {
				s_isUniChar.push(0);
			}
		}
		break;

	case WM_ESC:
		{
			if (vimProp->input_mode == vm_insert) {
				vimProp->input_mode = vm_command;
				if (vimProp->caret_pos.x != vimProp->caret_start_x) {
					ITextSelection* sel = NULL;
					HRESULT hr = vimProp->pDoc->get_Selection((IDispatch**)&sel);
					if (SUCCEEDED(hr) && sel != NULL) {
						CComVariant bExtend(false), nCount(1);
						hr = sel->CharLeft(bExtend, nCount);
					}
					::GetCaretPos(&vimProp->caret_pos);
					if (vimProp->caret_pos.x != vimProp->caret_start_x) {
						vimProp->caret_pos.x += 1;
						::SetCaretPos(vimProp->caret_pos.x, vimProp->caret_pos.y);
					}
				}
				Caret(hVim, vimProp);
			}
		}
		break;

	case WM_ARROWKEY:
		{
			::GetCaretPos(&vimProp->caret_pos);
			if (vimProp->caret_pos.x != vimProp->caret_start_x) {
				vimProp->caret_pos.x += 1;
				::SetCaretPos(vimProp->caret_pos.x, vimProp->caret_pos.y);
			}
			if (vimProp->input_mode == vm_command) {
				Caret(hVim, vimProp);
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

void Caret( HWND hWnd, PVIMProp vProp )
{
	TEXTMETRIC txtMetric;
	HDC hdc = ::GetDC(hWnd);
	if (::GetTextMetrics(hdc, &txtMetric)) {
		if (vProp->input_mode == vm_insert) {
			::DestroyCaret();
			::CreateCaret(hWnd, (HBITMAP)0, 2, txtMetric.tmHeight);
		}
		else { // vm_command, vm_command_line, vm_visual
			TCHAR ch = _T( 'a' );
			/* TODO: Get caret width */
			int nWidth;
			{
				POINT pt;
				ITextSelection* sel = NULL;
				HRESULT hr = vProp->pDoc->get_Selection((IDispatch**)&sel);
				if (SUCCEEDED(hr) && sel != NULL) {
					CComVariant bExtend(false), nCount(1);
					hr = sel->CharRight(bExtend, nCount);
					::GetCaretPos(&pt);
					hr = sel->CharLeft(bExtend, nCount);
				}
				if (pt.y == vProp->caret_pos.y && 
					pt.x != vProp->caret_pos.x) {
					if (vProp->caret_pos.x == vProp->caret_start_x) {
						nWidth = pt.x - vProp->caret_pos.x;
					}
					else {
						nWidth = pt.x - vProp->caret_pos.x + 1;
					}
				}
				else {
					::GetCharWidth32(hdc, ch, ch, &nWidth);
				}
			}
			::DestroyCaret();
			::CreateCaret(hWnd, (HBITMAP)0, nWidth, txtMetric.tmHeight);
		}
		::SetCaretPos(vProp->caret_pos.x, vProp->caret_pos.y);
		::ShowCaret(hWnd);
	}
}

void VimStart() {
	/* initialize g_BSTR_1Charactor_string */
	*(UINT32*)s_BSTR_1Charactor_string = 4U;
	s_ch = (BSTR)(s_BSTR_1Charactor_string + 4);
	::ZeroMemory((LPSTR)s_ch, 6);

	s_lobyte = s_hibyte = 0;

	s_wnd_working_child = NULL;
}

void s_TypeChar(PVIMProp vimProp) {
	ITextSelection* sel = NULL;
	if (vimProp->input_mode == vm_insert) {
		HRESULT hr = vimProp->pDoc->get_Selection((IDispatch**)&sel);
		if (SUCCEEDED(hr) && sel != NULL) {
			hr = sel->put_Text(s_ch);
			// record caret position
			::GetCaretPos(&vimProp->caret_pos);
			if (vimProp->caret_pos.x != vimProp->caret_start_x) {
				vimProp->caret_pos.x += 1;
				::SetCaretPos(vimProp->caret_pos.x, vimProp->caret_pos.y);
			}
		}
	}
}

void s_TypeCmdChar(PVIMProp vProp) {
	if (s_wnd_working_child != vProp->mdiChild) {
		s_wnd_working_child = vProp->mdiChild;
	}
}

void s_SetChar(DWORD ch) {
	::ZeroMemory((LPSTR)s_ch, 6);
	*(DWORD*)s_ch = ch;
}

void s_ClearChar() {
	::ZeroMemory((LPSTR)s_ch, 6);
}
