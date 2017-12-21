#pragma once

#include "MachinePattern.h"
#include "ScrollWnd.h"

// CEmptyWnd

class CEmptyWnd : public CScrollWnd
{
	DECLARE_DYNAMIC(CEmptyWnd)

public:
	CEmptyWnd();
	virtual ~CEmptyWnd();

	
	CMICallbacks *pCB;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


