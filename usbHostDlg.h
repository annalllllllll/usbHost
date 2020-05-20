
// usbHostDlg.h: 头文件
//

#pragma once


// CusbHostDlg 对话框
class CusbHostDlg : public CDialogEx
{
// 构造
public:
	CusbHostDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USBHOST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP();
	//BOOL DeviceOpen(HANDLE& handle, int iVID, int iPID);
	//void DeviceClose(HANDLE& handle);
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk();
	
	CString strDebug;
	HINSTANCE hDLL;
	BOOL thrExit;
	BOOL thrStateExit;
	CButton ledUSB;
	afx_msg void OnBnClickedButton3();
};
