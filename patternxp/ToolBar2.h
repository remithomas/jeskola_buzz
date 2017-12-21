#pragma once


// CToolBar2

class CToolBar2 : public CToolBar
{
	DECLARE_DYNAMIC(CToolBar2)

public:
	CToolBar2();
	virtual ~CToolBar2();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT CToolBar2::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
};


