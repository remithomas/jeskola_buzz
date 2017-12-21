#pragma once

#include "ScrollWnd.h"
#include "MachinePattern.h"

// CPatEd

class CPatEd : public CScrollWnd
{
	DECLARE_DYNAMIC(CPatEd)

public:
	CPatEd();
	virtual ~CPatEd();

	void Create(CWnd *parent);
	void SetPattern(CMachinePattern *p);

	void PatternLengthChanged(CMachinePattern *p);
	void SetPlayPos(CMachinePattern *pat, int pos);
	

	virtual void OnDraw(CDC *pDC);


private:
	CMachinePattern *pPlayingPattern;
	int playPos;
	int drawPlayPos;
	CPoint lastDrawPos;
	bool drawing;
	

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);


	CMICallbacks *pCB;
	CMachinePattern *pPattern;
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


