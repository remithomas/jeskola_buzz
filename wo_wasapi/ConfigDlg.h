#include "afxwin.h"
#if !defined(AFX_CONFIGDLG_H__8E063812_97DE_11D1_A298_2CC769000000__INCLUDED_)
#define AFX_CONFIGDLG_H__8E063812_97DE_11D1_A298_2CC769000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ConfigDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog

class CConfigDlg : public CDialog
{
// Construction
public:
	CConfigDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigDlg)
	enum { IDD = IDD_CONFIG };
	CComboBox	m_DeviceList;
	CComboBox	m_SampleRateBox;
	int		m_Device;
	//}}AFX_DATA

	int m_SampleRate;
	bool m_IsPolling;


	static void WriteLine(char const *txt);
	void AddText(char const *text);


	inline static void Write(char const *fmt, ...)
	{
		char buf[1024];

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf)-1, fmt, ap);
		va_end(ap);

		WriteLine(buf);
	}


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeConfigBufnum();
	afx_msg void OnChangeConfigBufsize();
	afx_msg void OnSelendokConfigSamplerate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void RecalcLatency();
public:
	CEdit m_Console;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CButton m_Shared;
	bool m_IsShared;
	CButton m_Poll;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__8E063812_97DE_11D1_A298_2CC769000000__INCLUDED_)
