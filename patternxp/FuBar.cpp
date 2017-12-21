// FuBar.cpp : implementation file
//

#include "stdafx.h"
#include "FuBar.h"


// CFuBar

IMPLEMENT_DYNAMIC(CFuBar, CReBar)

CFuBar::CFuBar()
{

}

CFuBar::~CFuBar()
{
}


BEGIN_MESSAGE_MAP(CFuBar, CReBar)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()


CSize CFuBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize s = CReBar::CalcFixedLayout(bStretch, bHorz);
	return s;
}


LRESULT CFuBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	if (IsWindowVisible())
    {
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
        if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
    }
	return 0L;
}
