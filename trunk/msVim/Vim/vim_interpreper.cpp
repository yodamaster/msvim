#include "stdafx.h"
#include "vim.h"

/* construct a one-character BSTR */
static BYTE s_BSTR_1Charactor_string[64];
static BSTR s_ch;
static void s_SetChar(OLECHAR ch);

/* input characters in the window */
static void s_TypeChar(PVIMProp vProp, OLECHAR bstr);

int VimInterpreter(HWND hVim, UINT msg, WPARAM wParam, LPARAM lParam, PVIMProp vimProp) {
	switch (msg) {
	case WM_CHAR:
		{
			OLECHAR ch = wParam;
			if ( (HIBYTE(HIWORD(lParam)) & 0x0001) == 1 ) {
				// ¿ì½Ý¼ü
				return -1;
			}
			else {
				// ASCII×Ö·û
				s_TypeChar(vimProp, ch);
			}
		}
		break;

	case WM_ESC:
		{
			if (vimProp->input_mode == vm_insert) {
				vimProp->input_mode = vm_command;
				ITextSelection* sel = NULL;
				HRESULT hr = vimProp->pDoc->get_Selection((IDispatch**)&sel);
				if (SUCCEEDED(hr) && sel != NULL) {
					CComVariant bExtend(false), nCount(1);
					hr = sel->CharLeft(bExtend, nCount);
				}
				::GetCaretPos(&vimProp->caret_pos);
				if (vimProp->caret_pos.x != vimProp->caret_start_x) {
					vimProp->caret_pos.x += 1;
				}
				::DestroyCaret();
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
		::DestroyCaret();
		if (vProp->input_mode == vm_insert) {
			::CreateCaret(hWnd, (HBITMAP)0, 2, txtMetric.tmHeight);
		}
		else { // vm_command, vm_command_line, vm_visual
			int nWidth;
			::GetCharWidth32(hdc, 'a', 'a', &nWidth);
			::CreateCaret(hWnd, (HBITMAP)0, nWidth, txtMetric.tmHeight);
		}
		::SetCaretPos(vProp->caret_pos.x, vProp->caret_pos.y);
		::ShowCaret(hWnd);
	}
}

void VimStart() {
	/* initialize g_BSTR_1Charactor_string */
	*(UINT32*)s_BSTR_1Charactor_string = 1U;
	s_ch = (BSTR)(s_BSTR_1Charactor_string + 4);
	*(SHORT*)(s_BSTR_1Charactor_string + 6) = 0;


}

void s_SetChar(OLECHAR ch) {
	*s_ch = ch;
}

void s_TypeChar(PVIMProp vimProp, OLECHAR ch) {
	ITextSelection* sel = NULL;
	if (vimProp->input_mode == vm_insert) {
		HRESULT hr = vimProp->pDoc->get_Selection((IDispatch**)&sel);
		if (SUCCEEDED(hr) && sel != NULL) {
			s_SetChar(ch);
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