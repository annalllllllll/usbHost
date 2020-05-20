
// usbHostDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "usbHost.h"
#include "usbHostDlg.h"
#include "afxdialogex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string> 
#include <Windows.h>
#include <iostream>
#include "resource.h"

extern "C"{
#include <hidsdi.h> 
#include <setupapi.h>  
#include <hidsdi.h>
};


using namespace std;

_declspec(dllimport) void __stdcall SETUSB_PID_VID(short p, short v);
_declspec(dllimport) int __stdcall USBScanDev(int NeedInit);
_declspec(dllimport) int __stdcall USBOpenDev(int DevIndex);
_declspec(dllimport) int __stdcall USBCloseDev(int DevIndex);
_declspec(dllimport) int __stdcall USBBulkReadData(unsigned int nBoardID, int pipenum, char* readbuffer, int len, int waittime);


typedef void(_stdcall *pSETUSB_PID_VID)(short p, short v);
typedef int(_stdcall* pUSBScanDev)(int NeedInit);
typedef int(__stdcall* pUSBOpenDev)(int DevIndex);
typedef int(__stdcall* pUSBCloseDev)(int DevIndex);
typedef int(__stdcall* pUSBBulkReadData)(unsigned int nBoardID, int pipenum, char* readbuffer, int len, int waittime);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CusbHostDlg 对话框



CusbHostDlg::CusbHostDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USBHOST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CusbHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CusbHostDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CusbHostDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CusbHostDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CusbHostDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON3, &CusbHostDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CusbHostDlg 消息处理程序

BOOL CusbHostDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	thrExit = TRUE;
	thrStateExit = TRUE;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CusbHostDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CusbHostDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CusbHostDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT usbThread(LPVOID pParam)
{
	CusbHostDlg* pDlg = (CusbHostDlg*)pParam;
	CString str, str_usb_data;
	int life_cycle = 0, rev_flag;

	//加载 DLL文件 
	pDlg->hDLL = LoadLibrary(L"USB_Driver.dll"); 
	if (pDlg->hDLL == NULL) {

		pDlg->thrExit = FALSE;
		pDlg->thrStateExit = FALSE;
		pDlg->strDebug += _T("DLL加载失败\r\n");
		::SetWindowText(::GetDlgItem(pDlg->m_hWnd, IDC_EDIT1), pDlg->strDebug);

		return 0;
	}
	pDlg->strDebug += _T("DLL加载成功\r\n");

	//设置指定PID VID
	pSETUSB_PID_VID SETUSB_PID_VID;
	SETUSB_PID_VID = (pSETUSB_PID_VID)GetProcAddress(pDlg->hDLL, "SETUSB_PID_VID");  //取DLL中的函数地址，以备调用  
	SETUSB_PID_VID(0x3405, 0x7653);
	pDlg->strDebug += _T("设置PID=0x3453&VID=0x7653完成\r\n");

	//查找设备
	pUSBScanDev USBScanDev;
	USBScanDev = (pUSBScanDev)GetProcAddress(pDlg->hDLL, "USBScanDev");  //取DLL中的函数地址，以备调用
	str.Format(_T("扫描到%d个设备\r\n"), USBScanDev(1));
	pDlg->strDebug += str;

	if (USBScanDev(1) == 0) {
		str.Format(_T("没有找到对应设备，关闭线程\r\n"), USBScanDev(1));

		//释放DLL
		pDlg->strDebug += _T("释放DLL\r\n");
		FreeLibrary(pDlg->hDLL);

		pDlg->thrExit = FALSE;
		pDlg->thrStateExit = TRUE;
		pDlg->strDebug += _T("子线程关闭成功\r\n");
		::SetWindowText(::GetDlgItem(pDlg->m_hWnd, IDC_EDIT1), pDlg->strDebug);

		return 0;
	}

	//打开usb
	pUSBOpenDev USBOpenDev;
	USBOpenDev = (pUSBOpenDev)GetProcAddress(pDlg->hDLL, "USBOpenDev");  //取DLL中的函数地址，以备调用
	
	if (SEVERITY_SUCCESS == USBOpenDev(0)) {		
		str = _T("打开成功\r\n");
	}
	else {
		pDlg->thrExit = FALSE;
		pDlg->thrStateExit = FALSE;
		str = _T("打开失败\r\n");

		//释放DLL
		pDlg->strDebug += _T("释放DLL\r\n");
		FreeLibrary(pDlg->hDLL);

		::SetWindowText(::GetDlgItem(pDlg->m_hWnd, IDC_EDIT1), pDlg->strDebug);

		return 0;
	}
	::SetWindowText(::GetDlgItem(pDlg->m_hWnd, IDC_EDIT1), pDlg->strDebug);
	while (pDlg->thrExit == FALSE) {

		pDlg->thrStateExit = FALSE;
		//Sleep(1000);

		//接收信息
		char readbuffer[64];
		memset(readbuffer, 0, sizeof(readbuffer));
		pUSBBulkReadData USBBulkReadData;
		USBBulkReadData = (pUSBBulkReadData)GetProcAddress(pDlg->hDLL, "USBBulkReadData");  //取DLL中的函数地址，以备调用
		rev_flag = USBBulkReadData(0, 0x81, readbuffer, 64, 0);

		switch (rev_flag)
		{
		case -5:
			str_usb_data.Format(_T("接收到-5报错啦 %d"), life_cycle++);
			break;
		default:
			break;
		}
		if ((rev_flag > -1)&&(readbuffer[0]))	{
			str_usb_data.Format(_T("接收到数据 %d"), life_cycle++); 
		}
		::SetWindowText(::GetDlgItem(pDlg->m_hWnd, IDC_EDIT3), str_usb_data);
		//发送信息

		//调试成功后开子线程

	}

	//关闭usb
	pUSBCloseDev USBCloseDev;
	USBCloseDev = (pUSBCloseDev)GetProcAddress(pDlg->hDLL, "USBCloseDev");  //取DLL中的函数地址，以备调用
	str = (SEVERITY_SUCCESS == USBCloseDev(0)) ? (_T("USB关闭成功\r\n")) : (_T("USB关闭失败\r\n"));
	pDlg->strDebug += str;

	//释放DLL
	pDlg->strDebug += _T("释放DLL\r\n");
	FreeLibrary(pDlg->hDLL);

	pDlg->thrStateExit = TRUE;
	pDlg->strDebug += _T("子线程关闭成功\r\n");
	::SetWindowText(::GetDlgItem(pDlg->m_hWnd, IDC_EDIT1), pDlg->strDebug);

	return 0;
}

void CusbHostDlg::OnBnClickedButton1()
{
	/*
	strDebug += _T("开始连接USB\r\n");
	HANDLE handle = NULL;

	if (DeviceOpen(handle, 0x7653, 0x3405) == FALSE)
	{
		strDebug += _T("没找到指定设备\r\n");
		SetDlgItemText(IDC_EDIT1, strDebug);
		return;
	}
	if (handle == INVALID_HANDLE_VALUE)
	{
		strDebug += _T("连接失败了呢\r\n");
		SetDlgItemText(IDC_EDIT1, strDebug);
		return;
	}

	strDebug += _T("连接成功\r\n");

	DeviceClose(handle);
	*/
	CString str = _T("");

	if (TRUE == thrStateExit)
	{	
		//开启子线程
		thrExit = FALSE;
		CWinThread* pWinThread;
		pWinThread = AfxBeginThread(usbThread, this);
		str = _T("开启子线程\r\n");
		strDebug += str;
	}
	else {
		str = _T("已经连接，不要再点啦(￢︿̫̿￢☆)\r\n");
		strDebug += str;
	}

	SetDlgItemText(IDC_EDIT1, strDebug);
}
/*
BOOL CusbHostDlg::DeviceOpen(HANDLE& handle, int iVID, int iPID) 
{
#pragma comment(lib, "setupapi.lib")
	CString vid, pid;
	//获取设备信息
	HDEVINFO hDevInfo = SetupDiGetClassDevs(
		NULL, 
		NULL, 
		NULL, 
		DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("获取系统设备列表失败"));
		return FALSE;
	}

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	deviceInfoData.DevInst = 0;
	deviceInfoData.Reserved = 0;

	DWORD regDataType;
	LPTSTR buffer = new TCHAR[200];
	DWORD buffersize = 200;

	for (int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); i++)
	{
		buffersize = 200;
		//The function retrieves a REG_SZ string that contains the device setup class of a device.
		SetupDiGetDeviceRegistryPropertyW(
			hDevInfo,
			&deviceInfoData,
			SPDRP_HARDWAREID,
			&regDataType,
			(PBYTE)buffer,
			buffersize,
			&buffersize);
		vid.Format(_T("VID_%x"), iVID);
		pid.Format(_T("PID_%x"), iPID);
		if (CString(buffer).MakeUpper().Find(vid + _T("&") + pid) > -1)
		{
			
			USHORT  RECV_DATA_LEN = 256;
			char recvDataBuf[256];
			DWORD recvdBytes;

			strDebug += (CString(buffer) + _T("\r\n报告长官！发现指定设备( •̀ ω •́ )y\r\n"));
			SetDlgItemText(IDC_EDIT1, strDebug);

			memset(recvDataBuf, 0, sizeof(recvDataBuf));
			if (!ReadFile(hDevInfo, recvDataBuf, RECV_DATA_LEN, &recvdBytes, NULL))
			{
				
			}

			SetupDiDestroyDeviceInfoList(hDevInfo);
			return TRUE;

		}
	}// end for
	if (buffer != nullptr)
	{
		delete[] buffer;
	}
	
	// 释放设备  
	SetupDiDestroyDeviceInfoList(hDevInfo);
	
	return TRUE;
}

void CusbHostDlg::DeviceClose(HANDLE& handle)
{
	CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
}
*/

void CusbHostDlg::OnBnClickedButton2()
{
	CString str = _T("");

	if (FALSE == thrStateExit) {
		//关闭子线程
		thrExit = TRUE;	
	}
	else {
		str = _T("关着呢，不要再点啦(￢︿̫̿￢☆)\r\n");
		strDebug += str;
	}
	
	SetDlgItemText(IDC_EDIT1, strDebug);
}


void CusbHostDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void CusbHostDlg::OnBnClickedButton3()
{
	strDebug = _T("");
	SetDlgItemText(IDC_EDIT1, strDebug);
}
