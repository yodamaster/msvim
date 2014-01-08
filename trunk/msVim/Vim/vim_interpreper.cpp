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

	case WM_KILLFOCUS:
		{
		}
		break;

	case WM_ESC:
		{
			if (vimProp->input_mode == vm_insert) {
				Caret(hVim, 0);
			}
		}
		break;

	default:
		break;
	}

	return 0;
}


void Caret( HWND hWnd, int caret )
{
	if (caret == 1) {
		::CreateCaret(hWnd, (HBITMAP)1, 0, 0);
		ShowCaret(hWnd);
		return;
	}

	CWnd* pWnd = CWnd::FromHandle(hWnd);
	if (pWnd != NULL)
	{
		CDC* pDC = pWnd->GetDC();

		CSize fontSize = pDC->GetTextExtent("1");
		::CreateCaret(hWnd, (HBITMAP)caret, fontSize.cx, fontSize.cy);
		ShowCaret(hWnd);

		pWnd->ReleaseDC(pDC);
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
		}
	}
}