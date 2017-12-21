// ToolBar2.cpp : implementation file
//

#include "stdafx.h"
#include "ToolBar2.h"


// CToolBar2

IMPLEMENT_DYNAMIC(CToolBar2, CToolBar)

CToolBar2::CToolBar2()
{

}

CToolBar2::~CToolBar2()
{
}


BEGIN_MESSAGE_MAP(CToolBar2, CToolBar)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()



CSize CToolBar2::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize s = CToolBar::CalcFixedLayout(bStretch, bHorz);
	return s;
}

LRESULT CToolBar2::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	if (IsWindowVisible())
    {
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
        if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
    }
	return 0L;
}
