#pragma once


// CFuBar

class CFuBar : public CReBar
{
	DECLARE_DYNAMIC(CFuBar)

public:
	CFuBar();
	virtual ~CFuBar();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);


protected:
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);

};


