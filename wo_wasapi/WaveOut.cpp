// waveout stuff based on old midas code

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600


//#define VC_EXTRALEAN
#define  _CRT_SECURE_NO_WARNINGS 

#define ISOLATION_AWARE_ENABLED 1

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxmt.h>

#include <afxcontrolbars.h>	// MFC support for ribbon and control bars

#include <afxadv.h>

#include <afxwin.h>
#include <afxcmn.h>
#include <mmsystem.h>
#include <stdio.h>
#include <memory.h>
#include <process.h>
#include <assert.h>
#include <vector>
#include <AudioClient.h>
#include <Mmdeviceapi.h>
#include <audiopolicy.h>
#include <avrt.h>
#include <functiondiscoverykeys.h>
#include <strsafe.h>

#include "../audiodriver/AudioDriver.h"
#include "resource.h"
#include "ConfigDlg.h"

#define AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED      AUDCLNT_ERR(0x019)

std::vector<CString> DeviceNames;
	IMMDeviceCollection *deviceCollection = NULL;

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

CString GetDeviceName(IMMDeviceCollection *DeviceCollection, UINT DeviceIndex)
{
    IMMDevice *device;
    LPWSTR deviceId;
    HRESULT hr;

    hr = DeviceCollection->Item(DeviceIndex, &device);
    if (FAILED(hr))
    {
        printf("Unable to get device %d: %x\n", DeviceIndex, hr);
        return "";
    }
    hr = device->GetId(&deviceId);
    if (FAILED(hr))
    {
        printf("Unable to get device %d id: %x\n", DeviceIndex, hr);
        return "";
    }

    IPropertyStore *propertyStore;
    hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
    SafeRelease(&device);
    if (FAILED(hr))
    {
        printf("Unable to open device %d property store: %x\n", DeviceIndex, hr);
        return "";
    }

    PROPVARIANT friendlyName;
    PropVariantInit(&friendlyName);
    hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
    SafeRelease(&propertyStore);

    if (FAILED(hr))
    {
        printf("Unable to retrieve friendly name for device %d : %x\n", DeviceIndex, hr);
        return "";
    }

    wchar_t deviceName[128];
    hr = StringCbPrintfW(deviceName, sizeof(deviceName), L"%s", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal);
    if (FAILED(hr))
    {
        printf("Unable to format friendly name for device %d : %x\n", DeviceIndex, hr);
        return "";
    }

    PropVariantClear(&friendlyName);
    CoTaskMemFree(deviceId);

	return deviceName;
}

void GetDeviceList()
{
	HRESULT hr;
	bool retValue = true;
	IMMDeviceEnumerator *deviceEnumerator = NULL;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
	if (FAILED(hr))
		return;

	hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
	if (FAILED(hr))
		return;

	UINT deviceCount;
	hr = deviceCollection->GetCount(&deviceCount);
	if (FAILED(hr))
		return;

	for (UINT i = 0 ; i < deviceCount ; i += 1)
	{
		 CString deviceName = GetDeviceName(deviceCollection, i);
		
		 if (deviceName.GetLength() == 0)
			 return;

		 DeviceNames.push_back(deviceName);
	}
}


#define MFTIMES_PER_MILLISEC  10000

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

class ad : public CAudioDriver
{
public:
	ad();
	virtual ~ad();
	virtual void Initialize(dword hwnd, float *(*pcallback)(int &numsamples));
	virtual void Reset();
	virtual bool Enable(bool e);	
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual void Configure();

	void DoBlocks();

private:
	bool Start();	
	bool Stop();
	void ReadConfig();
	void WriteConfig();
	void Error(char const *msg);

	void RenderSamples(void *pbuf, int numsamples);

public:
	void AudioThread();

public:
	HANDLE hAudioThread;
	int DeviceID;
	float *(*pCallback)(int &numsamples);
	IMMDeviceEnumerator *pEnumerator;
	HANDLE hEvent;
	IAudioClient *pAudioClient;
	volatile bool killAudioThread;

	bool Initialized;
	bool Running;

	int Shared;
	int Poll;

	bool FloatMode;

};

ad::ad()
{
	CConfigDlg::Write("*** CAudioDriver::CAudioDriver ***");
	Initialized = false;
	hAudioThread = NULL;
	pEnumerator = NULL;
	hEvent = NULL;
	pAudioClient = NULL;
	Shared = 1;
	Poll = 1;

	GetDeviceList();
}


ad::~ad()
{
	CConfigDlg::Write("*** CAudioDriver::~CAudioDriver ***");
	Reset();
}

void ad::Error(char const *msg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//	MessageBox(NULL, msg, "WASAPI", MB_OK);

	CConfigDlg::WriteLine(msg);
}


unsigned int __stdcall AudioThreadProc(void *pt)
{
	ad *pad = (ad *)pt;
	pad->AudioThread();
	return 0;
}

inline short ConvertSample(float f)
{
	int x = (int)f;
	if (x < -32768)
		return -32768;
	else if (x > 32767)
		return 32767;
	else
		return (short)x;
}

inline float ConvertSampleF(float x)
{
	if (x < -32768.0f)
		return -32768.0f;
	else if (x > 32767.0f)
		return 32767.0f;
	else
		return x;
}

void ad::RenderSamples(void *output, int numsamples)
{
	if (FloatMode)
	{
		float *pout = (float *)output;

		do
		{
			int n = numsamples;
			float *pbuf = pCallback(n);

			for (int i = 0; i < n; i++)
			{
				*pout++ = ConvertSampleF(pbuf[2*i+0]) * (1.0f / 32768.0f);
				*pout++ = ConvertSampleF(pbuf[2*i+1])  * (1.0f / 32768.0f);
			}

			numsamples -= n;
		} while(numsamples > 0);
	}
	else
	{
		short *pout = (short *)output;

		do
		{
			int n = numsamples;
			float *pbuf = pCallback(n);

			for (int i = 0; i < n; i++)
			{
				*pout++ = ConvertSample(pbuf[2*i+0]);
				*pout++ = ConvertSample(pbuf[2*i+1]);
			}

			numsamples -= n;
		} while(numsamples > 0);
	}
}

void ad::AudioThread()
{
	HRESULT hr;


	if (!Poll)
	{
		hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hEvent == NULL)	Error("CreateEvent failed");

		hr = pAudioClient->SetEventHandle(hEvent);
		if (hr != S_OK)	Error("IAudioClient::SetEventHandle failed");
	}

	UINT bufferSize = 0;
	hr = pAudioClient->GetBufferSize(&bufferSize);
	if (hr != S_OK)	
		Error("IAudioClient::GetBufferSize failed");
	else
		CConfigDlg::Write("GetBufferSize: %d\n", bufferSize);

	REFERENCE_TIME lat;
	pAudioClient->GetStreamLatency(&lat);
	int latms = (int)(lat / MFTIMES_PER_MILLISEC);
	CConfigDlg::Write("GetStreamLatency: %dms\n", latms);

	IAudioRenderClient *pRenderClient = NULL;
	hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);
	if (hr != S_OK)	Error("IAudioClient::GetService failed");

	BYTE *pData;
	
/*
	//for (int i = 0; i < 2; i++)
	{
		hr = pRenderClient->GetBuffer(bufferSize, &pData);
		if (hr != S_OK)	Error("IRenderClient::GetBuffer failed");

		hr = pRenderClient->ReleaseBuffer(bufferSize, AUDCLNT_BUFFERFLAGS_SILENT);
		if (hr != S_OK)	Error("IRenderClient::ReleaseBuffer failed");

	}
*/	

	DWORD taskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
	if (hTask == NULL) Error("Couldn't set Pro Audio priority");


	hr = pAudioClient->Start();  
	if (hr != S_OK)	Error("IAudioClient::Start failed");


	killAudioThread = false;

	while(!killAudioThread)
	{
		if (Poll)
		{
			Sleep(1);
		}
		else
		{

			DWORD retval = WaitForSingleObject(hEvent, 2000);
			if (retval != WAIT_OBJECT_0)
			{
				Error("WaitForSingleObject timed out");
				goto exit;
			}
		}
	


//		UINT available = bufferSize;

		
        UINT32 padding = 0;

		if (Shared || Poll)
		{
			hr = pAudioClient->GetCurrentPadding(&padding);
			if (hr != S_OK)
			{
				Error("IAudioClient::GetCurrentPadding failed");
				goto exit;
			}
			else
			{
	//			CConfigDlg::Write("GetCurrentPadding: %d\n", padding);
			
			}
		}

        
		int available = bufferSize - padding;


		if (available >= 0)
		{
			hr = pRenderClient->GetBuffer(available, &pData);
			if (hr != S_OK)
			{
				Error("IRenderClient::GetBuffer failed");
				goto exit;
			}

			RenderSamples(pData, available);

			hr = pRenderClient->ReleaseBuffer(available, 0);
			if (hr != S_OK)	
			{
				Error("IRenderClient::ReleaseBuffer failed");
				goto exit;
			}
		}


	}


exit:
	SAFE_RELEASE(pRenderClient);

}

bool ad::Start()
{
	if (Running)
		return true;

	FloatMode = false;

	CConfigDlg::Write("*** CAudioDriver::Start ***");

	IMMDevice *pDevice = NULL;
	HRESULT hr;

	if (DeviceID == 0)
	{
		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);	
		if (hr != S_OK)	
		{
			Error("IMMDeviceEnumerator::GetDefaultAudioEndpoint failed");
			return false;
		}

	}
	else
	{
		hr = deviceCollection->Item(DeviceID - 1, &pDevice);
		if (hr != S_OK)	
		{
			Error("IMMDeviceCollection::Item failed");
			return false;
		}
	}

	hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (hr != S_OK)	
	{
		Error("IMMDevice::Activate failed");
		return false;
	}

	
	WAVEFORMATEX *pwfx = NULL;	
	//hr = GetStreamFormat(pAudioClient, &pwfx);
	hr = pAudioClient->GetMixFormat(&pwfx);
	if (hr != S_OK)	
	{
		Error("IAudioClient::GetMixFormat failed");
		return false;
	}
	else
	{
		CConfigDlg::Write("GetMixFormat: sr %d bps %d nch %d", pwfx->nSamplesPerSec, pwfx->wBitsPerSample, pwfx->nChannels); 
	}

	if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE *p = (WAVEFORMATEXTENSIBLE *)pwfx;
		if (p->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
		{
		}
		
		
	}

	WAVEFORMATEX *cmfmt = NULL;

	hr = pAudioClient->IsFormatSupported(Shared != 0 ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE, pwfx, &cmfmt);
	
	if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT)
	{
		if (!Shared)
			CConfigDlg::Write("MixFormat is not supported in exclusive mode"); 
	}
	else if (hr != S_OK)	
	{
		Error("IAudioClient::IsFormatSupported failed");
		return false;
	}
	else
	{
		if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			FloatMode = true;
			CConfigDlg::Write("Using WAVE_FORMAT_IEEE_FLOAT"); 
		}
		else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			WAVEFORMATEXTENSIBLE *p = (WAVEFORMATEXTENSIBLE *)pwfx;
			if (p->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
			{
				FloatMode = true;
				CConfigDlg::Write("Using WAVE_FORMAT_EXTENSIBLE, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT"); 
			}
		}

		// TODO: try IsFormatSupported first
//		FloatMode = false;
	}

	REFERENCE_TIME defLatency = 0, minLatency = 0;
	hr = pAudioClient->GetDevicePeriod(&defLatency, &minLatency);
	if (hr != S_OK)	
	{
		Error("IAudioClient::GetDevicePeriod failed");
		return false;
	}
	else
		CConfigDlg::Write("GetDevicePeriod: def %dms min %dms", (int)defLatency / MFTIMES_PER_MILLISEC, (int)minLatency / MFTIMES_PER_MILLISEC);


	WAVEFORMATEXTENSIBLE wfext;
	WAVEFORMATEX &wfex = wfext.Format;
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = 2;
	wfex.wBitsPerSample = 16;
	wfex.nSamplesPerSec = SamplesPerSec;
	wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.cbSize = 0;

	if (FloatMode)
	{
		if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)		
			memcpy(&wfext, pwfx, sizeof(WAVEFORMATEX));
		else
			memcpy(&wfext, pwfx, sizeof(WAVEFORMATEXTENSIBLE));

	}

	SamplesPerSec = wfex.nSamplesPerSec;

	REFERENCE_TIME latency = minLatency;

	hr = pAudioClient->Initialize(Shared != 0 ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE, Poll != 0 ? 0 : AUDCLNT_STREAMFLAGS_EVENTCALLBACK, latency, latency, &wfex, NULL);
	if (hr != S_OK)	
	{
		if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
		{
			UINT bufferSize = 0;
			hr = pAudioClient->GetBufferSize(&bufferSize);
			if (hr != S_OK)	
			{
				Error("IAudioClient::GetBufferSize failed");
				return false;
			}
			else
			{
				CConfigDlg::Write("GetBufferSize: %d\n", bufferSize);
			}

			if ((bufferSize * wfex.nBlockAlign % 128) != 0)
			{
				Error("Invalid buffer size");
				return false;
			}

			 REFERENCE_TIME bufferDuration = (REFERENCE_TIME)(10000.0 *                         // (REFERENCE_TIME / ms) *
											   1000 *                            // (ms / s) *
											   bufferSize /                      // frames /
											   wfex.nSamplesPerSec +      // (frames / s)
											   0.5);                             // rounding
			
			 CConfigDlg::Write("Adjusting latency from %.1fms to %.1fms\n", (float)minLatency / MFTIMES_PER_MILLISEC, (float)bufferDuration / MFTIMES_PER_MILLISEC);

			hr = pAudioClient->Initialize(Shared != 0 ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE, Poll != 0 ? 0 : AUDCLNT_STREAMFLAGS_EVENTCALLBACK, bufferDuration, bufferDuration, &wfex, NULL);
			if (hr != S_OK)	
			{
				Error("IAudioClient::Initialize failed");
				return false;
			}
			else
				CConfigDlg::Write("Initialize: sr %d bps %d nch %d", wfex.nSamplesPerSec, wfex.wBitsPerSample, wfex.nChannels);


		}
		else
		{
			Error("IAudioClient::Initialize failed");
			return false;
		}

	}
	else
		CConfigDlg::Write("Initialize: sr %d bps %d nch %d", wfex.nSamplesPerSec, wfex.wBitsPerSample, wfex.nChannels);

	hAudioThread = (HANDLE)_beginthreadex(NULL, 0, AudioThreadProc, this, 0, NULL);


	Running = true;
	return true;
}

bool ad::Stop()
{
	if (!Running)
		return true;

	CConfigDlg::Write("*** CAudioDriver::Stop ***");

	killAudioThread = true;

	if (WaitForSingleObject(hAudioThread, 5000) == WAIT_TIMEOUT)
		Error("Audio Thread did not terminate properly");

	hAudioThread = NULL;
	
	Sleep(100);

	HRESULT hr = pAudioClient->Stop();
	if (hr != S_OK)	
		Error("IAudioClient::Stop failed");

	hr = pAudioClient->Reset();
	if (hr != S_OK)	
		Error("IAudioClient::Reset failed");

	SAFE_RELEASE(pAudioClient);

	if (hEvent != NULL)
	{
		CloseHandle(hEvent);
		hEvent = NULL;
	}

	Running = false;
	return true;
}

void ad::ReadConfig()
{
	CConfigDlg::Write("*** CAudioDriver::ReadConfig ***");

	SamplesPerSec = ReadProfileInt("SamplesPerSec", 44100);
	DeviceID = ReadProfileInt("DeviceID", 0);
	Shared = ReadProfileInt("Shared", 1);
	Poll = ReadProfileInt("Poll", 0);

	/*
	numBlocks = ReadProfileInt("numBlocks", 4);
	BlockSize = ReadProfileInt("BlockSize", 8192);
	PollSleep = ReadProfileInt("PollSleep", 20);
	Dither = ReadProfileInt("Dither", 0);
*/

	Flags = ADF_STEREO;
}

void ad::WriteConfig()
{
	CConfigDlg::Write("*** CAudioDriver::WriteConfig ***");

	WriteProfileInt("SamplesPerSec", SamplesPerSec);
	WriteProfileInt("DeviceID", DeviceID);
	WriteProfileInt("Shared", Shared);
	WriteProfileInt("Poll", Poll);

	/*
	WriteProfileInt("numBlocks", numBlocks);
	WriteProfileInt("BlockSize", BlockSize);
	WriteProfileInt("PollSleep", PollSleep);
	WriteProfileInt("Dither", Dither);
	*/
}

void ad::Initialize(dword hwnd, float *(*pcallback)(int &numsamples))
{
	CConfigDlg::Write("*** CAudioDriver::Initialize ***");

	pCallback = pcallback;
	Running = false;
	ReadConfig();

	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (hr != S_OK)	Error("CoCreateInstance CLSID_MMDeviceEnumerator failed");


	Initialized = true;
	Start();
}

void ad::Reset()
{
	if (!Initialized)
		return;

	Stop();

	CConfigDlg::Write("*** CAudioDriver::Reset ***");
	

	Initialized = false;
}

void ad::Configure()
{
	CConfigDlg::Write("*** CAudioDriver::Configure ***");

	int oldsps, olddid, oldshared, oldpoll;
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CConfigDlg dlg;
		dlg.m_SampleRate = SamplesPerSec;
		dlg.m_Device = DeviceID;
		dlg.m_IsShared = Shared != 0;
		dlg.m_IsPolling = Poll != 0;
			
		{

			if (dlg.DoModal() != IDOK)
				return;
		}

		oldsps = SamplesPerSec;	
		olddid = DeviceID;	
		oldshared = Shared;
		oldpoll = Poll;

		if (Initialized)
			Stop();

		SamplesPerSec = dlg.m_SampleRate;
		DeviceID = dlg.m_Device;
		Shared = dlg.m_IsShared ? 1 : 0;
		Poll = dlg.m_IsPolling ? 1 : 0;
	}

	if (Initialized)
	{
		if (Start())
		{
			WriteConfig();
		}
		else
		{
			CConfigDlg::Write("Revert to previous config");

			SamplesPerSec = oldsps;
			DeviceID = olddid;
			Shared = oldshared;
			Poll = oldpoll;

			Start();
		}
	}
	else
	{
		WriteConfig();
	}
}

int ad::GetPlayPos()
{
	return 0;


}

int ad::GetWritePos()
{
	return 0;

}

bool ad::Enable(bool e)
{
	CConfigDlg::Write("*** CAudioDriver::Enable ***");

	if (e)
		return Start();
	else
		return Stop();
}

CAudioDriverInfo info = { 
//#ifdef _DEBUG
//	"WASAPI (debug build)"
//#else
	"WASAPI"
//#endif
};

extern "C"
{
__declspec(dllexport) CAudioDriver * __cdecl NewAD()
{
	return new ad;
}

__declspec(dllexport) CAudioDriverInfo const * __cdecl GetADInfo()
{
	return &info;
}
}

CWinApp App;	