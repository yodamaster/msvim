// DSAddIn.h : header file
//

#if !defined(AFX_DSADDIN_H__A5E14AA7_C2AC_4601_81C8_4E2A60B5253C__INCLUDED_)
#define AFX_DSADDIN_H__A5E14AA7_C2AC_4601_81C8_4E2A60B5253C__INCLUDED_

#include "commands.h"

// {DE4A32FA-D760-4E79-B66E-D5D384F34C05}
DEFINE_GUID(CLSID_DSAddIn,
0xde4a32fa, 0xd760, 0x4e79, 0xb6, 0x6e, 0xd5, 0xd3, 0x84, 0xf3, 0x4c, 0x5);

/////////////////////////////////////////////////////////////////////////////
// CDSAddIn

class CDSAddIn : 
	public IDSAddIn,
	public CComObjectRoot,
	public CComCoClass<CDSAddIn, &CLSID_DSAddIn>
{
public:
	DECLARE_REGISTRY(CDSAddIn, "MsVim.DSAddIn.1",
		"MSVIM Developer Studio Add-in", IDS_MSVIM_LONGNAME,
		THREADFLAGS_BOTH)

	CDSAddIn() {}
	BEGIN_COM_MAP(CDSAddIn)
		COM_INTERFACE_ENTRY(IDSAddIn)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CDSAddIn)

// IDSAddIns
public:
	STDMETHOD(OnConnection)(THIS_ IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection);
	STDMETHOD(OnDisconnection)(THIS_ VARIANT_BOOL bLastTime);

protected:
	CCommandsObj* m_pCommands;
	DWORD m_dwCookie;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSADDIN_H__A5E14AA7_C2AC_4601_81C8_4E2A60B5253C__INCLUDED)
