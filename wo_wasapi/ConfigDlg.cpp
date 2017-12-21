// ConfigDlg.cpp : implementation file
//

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600


#define VC_EXTRALEAN
#define  _CRT_SECURE_NO_WARNINGS 

#include <afxwin.h>
#include <afxcmn.h>
#include <afxmt.h>
#include <mmsystem.h>
#include <queue>
#include "resource.h"
#include "ConfigDlg.h"

extern std::vector<CString> DeviceNames;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

static CString ctext;

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog


CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
	, m_IsShared(false)
{
	//{{AFX_DATA_INIT(CConfigDlg)
	m_Device = -1;
	//}}AFX_DATA_INIT
}


void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDlg)
	DDX_Control(pDX, IDC_CONFIG_DEVICE, m_DeviceList);
	DDX_Control(pDX, IDC_CONFIG_SAMPLERATE, m_SampleRateBox);
	DDX_CBIndex(pDX, IDC_CONFIG_DEVICE, m_Device);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CONSOLE, m_Console);
	DDX_Control(pDX, IDCANCEL, m_Shared);
	DDX_Control(pDX, IDC_POLL, m_Poll);
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CConfigDlg)
	ON_CBN_SELENDOK(IDC_CONFIG_SAMPLERATE, OnSelendokConfigSamplerate)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

#define MIN_NBUF		2
#define MAX_NBUF		8
#define MIN_SBUF		512
#define MAX_SBUF		(32768 - 512)

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg message handlers

BOOL CConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// device list
	{
		/*
		PaDeviceIndex dc = Pa_GetDeviceCount();

		int index = 0;

		for (int i = 0; i < dc; i++)
		{
			PaDeviceInfo const *di = Pa_GetDeviceInfo(i);
			PaHostApiInfo const *hai = Pa_GetHostApiInfo(di->hostApi);

			CString han = hai->name;
			if (han == "Windows WASAPI")
				han = "WASAPI";
			else if (han == "Windows DirectSound")
				han = "DS";

			CString s;
			s.Format("[%s] %s", han, di->name);

			if (di->maxOutputChannels >= 2)
			{
				if (index == m_Device)
				{
					CString s;
					s.Format("Latency: %dms", (int)(di->defaultLowOutputLatency * 1000.0));
					m_Latency.SetWindowText(s);
				}

				m_DeviceList.AddString(s);
				index++;
			}

			
		}

		if (m_Device >= index)
			m_Device = 0;
		*/

		m_DeviceList.SetCurSel(0);

//		m_DeviceList.SetCurSel(m_Device);

		/*
		if (m_stream != NULL)
		{
			PaStreamInfo const *psi = Pa_GetStreamInfo(m_stream);
				CString s;
				s.Format("Latency: %dms ", (int)(psi->outputLatency * 1000.0));
				m_Latency.SetWindowText(s);
		}
		*/


		int n = DeviceNames.size();
	
		for (int i = 0; i < n; i++)
			m_DeviceList.AddString(DeviceNames[i]);
			
		if (m_Device < 0 || m_Device > n)
			m_Device = 0;

		m_DeviceList.SetCurSel(m_Device);
		
	}

	// samplerate
	{
		CString str;
		str.Format("%d", m_SampleRate);
		
		int i = m_SampleRateBox.SelectString(-1, str);
		if (i == CB_ERR)
			i = m_SampleRateBox.SelectString(-1, "44100");
	}

	if (m_IsShared)
		CheckRadioButton(IDC_SHARED, IDC_EXCLUSIVE, IDC_SHARED);
	else
		CheckRadioButton(IDC_SHARED, IDC_EXCLUSIVE, IDC_EXCLUSIVE);

	m_Poll.SetCheck(m_IsPolling ? BST_CHECKED : BST_UNCHECKED);

	m_Console.SetWindowText(ctext);

	SetTimer(1, 100, NULL);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigDlg::OnOK() 
{
	CString str;
	m_SampleRateBox.GetWindowText(str);
	m_SampleRate = atoi(str);

	
	m_IsShared = GetCheckedRadioButton(IDC_SHARED, IDC_EXCLUSIVE) == IDC_SHARED ? 1 : 0;
	
	m_IsPolling = m_Poll.GetCheck() == BST_CHECKED;

	CDialog::OnOK();
}



void CConfigDlg::OnSelendokConfigSamplerate() 
{
	if (!IsWindow(m_SampleRateBox.GetSafeHwnd()))
		return;
	
}



static queue<CString> consoleQueue;
static CCriticalSection cs;

void CConfigDlg::WriteLine(char const *txt)
{
	CSingleLock lock(&cs);
	consoleQueue.push(txt);
}

void CConfigDlg::OnTimer(UINT_PTR nIDEvent)
{
	vector<CString> strs;
	{
		CSingleLock lock(&cs);
		while(!consoleQueue.empty())	
		{
			strs.push_back(consoleQueue.front());
			consoleQueue.pop();
		}
	}

	CString all = "";

	for (int i = 0; i < (int)strs.size(); i++)
	{
		all += strs[i];
		all += "\r\n";
	}
		
	if (all.GetLength() > 0)
		AddText(all);

	CDialog::OnTimer(nIDEvent);
}


void CConfigDlg::AddText(char const *text)
{
	int len = m_Console.GetWindowTextLength();
	m_Console.SetSel(len, len);
	m_Console.ReplaceSel(text);
	m_Console.SetSel(-1, 0);
}

void CConfigDlg::OnDestroy()
{
	CDialog::OnDestroy();

	m_Console.GetWindowText(ctext);
}
