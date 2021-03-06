// FTPclient.cpp : implementation file
//

#include "stdafx.h"
#include "通信.h"
#include "FTPclient.h"
#include "ClassXP.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFTPclient dialog


CFTPclient::CFTPclient(CWnd* pParent /*=NULL*/)
	: CDialog(CFTPclient::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFTPclient)
	m_address = _T("");
	m_user = _T("");
	m_password = _T("");
	m_port = 0;
	m_state = _T("");
	//}}AFX_DATA_INIT
	m_address="ftp.itmy520.com";
	m_user="down";
	m_password="down";
	m_port=21;
	m_bConnected=FALSE;
	m_bDir=FALSE;
}


void CFTPclient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFTPclient)
	DDX_Control(pDX, IDC_LIST1, m_listdir);
	DDX_Text(pDX, IDC_HOST_ADDRESS, m_address);
	DDX_Text(pDX, IDC_USER, m_user);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDX_Text(pDX, IDC_STATTE, m_state);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFTPclient, CDialog)
	//{{AFX_MSG_MAP(CFTPclient)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_UPLOAD, OnUpload)
	ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_DISCONNECT, OnDisconnect)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, OnSetfocusList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFTPclient message handlers

void CFTPclient::OnConnect() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString strServerName;  //服务器名
	CString strObject;     //查询对象名
	INTERNET_PORT nPort;
	DWORD dwServiceType;   //服务类型
	CWaitCursor cursor;
	if(!AfxParseURL(m_address,dwServiceType,strServerName,strObject,nPort))  //解析该地址
	{    
	 CString strFtpURL=_T("ftp://");  //解析不成功在地址前加ftp://
	 strFtpURL+=m_address;
	 if(!AfxParseURL(strFtpURL,dwServiceType,strServerName,strObject,nPort))
	 {  //再次解析,还不成功,则输入的地址错误
		 MessageBox("无效地址!",MB_OK);
		 return;
     }
	}
	//如果该地址提供ftp服务,且服务器名称不为空时,进行连接操作
	if((dwServiceType==INTERNET_SERVICE_FTP)&&!strServerName.IsEmpty())
	{
	 try
	 {  //向服务器发送请求,建立连接
	  pFtpConnection=pInetSession.GetFtpConnection(strServerName,m_user,m_password,m_port,TRUE);
	 }
	 catch(CInternetException* pEx)
	   { pEx->Delete();
	      pFtpConnection=NULL;
	   }
	}
	else
	{  //该网络地址不提供FTP服务
		MessageBox("无效FTP网络地址",MB_OK);
	  }
	  //如连接成功,更新列表视图
	if(pFtpConnection!=NULL)
	{
	  m_bConnected=TRUE;
	  m_state="连接成功";
	  CString CurrentDirectory;
	  pFtpConnection->GetCurrentDirectory(CurrentDirectory);  //得到连接的当前目录
	  List(CurrentDirectory);   //列表内容
	}
	else
	{
	  m_state="连接失败";
	}
	UpdateData(FALSE);
}

void CFTPclient::OnUpload() 
{
	// TODO: Add your control notification handler code here
	if(m_bConnected)
	{
	 CString pathname;
	 CFileDialog dlg(true);  //打开文件对话框
	 if(dlg.DoModal()==IDOK)
	 {
	   pathname=dlg.GetPathName();  //得到上传文件的路径
	 }
	 
      pFtpConnection->PutFile(pathname,pathname);  //上传文件
	}
}

void CFTPclient::OnDownload() 
{
	// TODO: Add your control notification handler code here
	if(m_bConnected)
	{
	 if(!m_bDir)
	 {  //选择为文件
		 CString pathname;
		 CFileDialog dlg(false);  //保存文件对话框
		 char fileme[500]={0};
		 strcpy(fileme,m_filename);
		 dlg.m_ofn.lpstrFile=fileme;  //保存到本地的文件名
          if(dlg.DoModal()==IDOK)
		  {
		    pathname=dlg.GetPathName(); //得到保存到本地的文件路径
		  }  //m_filename为鼠标选中的服务器上的文件名,由IsDir()得到
		  pFtpConnection->GetFile(m_filename,pathname);
	 }
	 else {MessageBox("选择为子目录,不能下载");
	 }
	}
}

void CFTPclient::OnDisconnect() 
{
	// TODO: Add your control notification handler code here
	if(m_bConnected)
	{
	 pFtpConnection->Close();  //关闭与服务器的连接
      m_bConnected=FALSE;
	  m_state="断开连接";
	  UpdateData(FALSE);
	}
}

void CFTPclient::OnUp() 
{
	// TODO: Add your control notification handler code here
	if(m_bConnected)
	{
	  CString CurDir;
	  pFtpConnection->GetCurrentDirectory(CurDir);  //得到当前目录
	   if(CurDir=="/")
	   {
	    MessageBox("位于根目录","错误");
		return;
	   }
	   int pos=CurDir.ReverseFind('/');  //从字符串末端开始查找字符'/'
       if(pos==-1)
	  {
	   MessageBox("不能返回上级目录","错误");
	   return;
	  }
	   CString updir;
	   updir=CurDir.Left(pos);  //得到上级目录名
	   if(pos==0)
		   updir="/";
	       List(updir);  //列目录
	}
}

void CFTPclient::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{    //该函数对应鼠标双击列表视图控件消息
	// TODO: Add your control notification handler code here
	CWaitCursor cursor;
	if(m_listdir.GetSelectedCount()!=1)
		return;
	POSITION pos=m_listdir.GetFirstSelectedItemPosition();
	UINT nIndex=m_listdir.GetNextSelectedItem(pos);
	CString strName=m_listdir.GetItemText(nIndex,0); //获得列表视图选中项的名字
	if(IsDir(strName))  //如为文件  返回
	{
	 List(strName);  //如为子目录,双击列表项列出子目录内容
	}
	else {
	 OnDownload();  //如为文件,双击列表项下载文件
	}
	*pResult = 0;
}

void CFTPclient::OnSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult) 
{        //该函数响应列表视图项的焦点事件
	// TODO: Add your control notification handler code here
	if(m_bConnected)
	{
	 if(m_listdir.GetSelectedCount()!=1)
		 return;
	 //获得列表视图选中项的名字
	 POSITION pos=m_listdir.GetFirstSelectedItemPosition();
	 UINT nIndex=m_listdir.GetNextSelectedItem(pos);
	 CString strName=m_listdir.GetItemText(nIndex,0);
	 m_bDir=IsDir(strName);
	}
	*pResult = 0;
}

BOOL CFTPclient::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	CImageList* pImageList=new CImageList();
	pImageList->Create(15,15,TRUE,1,2);  //创建图像列表
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_DIR);     //子目录对应图像
	pImageList->Add(&bitmap,(COLORREF)0xFFFFFF);  //在图像列表中增加图像
     bitmap.DeleteObject();
	 bitmap.LoadBitmap(IDB_FILE);  //文件对应图像
	 pImageList->Add(&bitmap,(COLORREF)0xFFFFFF);  //在图像列表中增加图像
    bitmap.DeleteObject();
	m_listdir.SetImageList(pImageList,LVSIL_NORMAL);  //指定一个图像列表到列表视图控件
	pFtpConnection=NULL;
	pImageList->Detach();    //把图像列表与CImagelist对象分离
//	ClassXP(NULL,TRUE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFTPclient::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pFtpConnection!=NULL)
	{
	  pFtpConnection->Close();
	  delete pFtpConnection;
	}
	return CDialog::DestroyWindow();
}

BOOL CFTPclient::IsDir(CString dir)  //判断在列表视图中是选择文件还是子目录
{
   BOOL direc=FALSE;
   CFtpFileFind finder(pFtpConnection);
   BOOL bWorking=finder.FindFile("*");  //*代表查找所有文件
   while(bWorking)
   {
	   bWorking=finder.FindNextFile();  //查找下一个文件
       if(strcmp(dir,finder.GetFileName())==0)
	   {
		    if(finder.IsDirectory())   //为子目录
			{
				direc=TRUE;
				break;
			}
			else
			{
			  m_filename=finder.GetFileName();
			}
	   }
   }
      finder.Close();
	  return(direc);
}

void CFTPclient::List(CString strDir)  //用于列出所选子目录的内容(子目录和文件)
{ 
	CString str;
	CWaitCursor cursor;   //显示等待光标
	CString filename;
	int strFlag;
	int Index=0;
	CString CurrentDirectory;
	pFtpConnection->GetCurrentDirectory(CurrentDirectory);  //获取此次连接的当前目录
	if(!pFtpConnection->SetCurrentDirectory(strDir)) //设置FTP当前目录为strDir
	{
      str="目录不能改变";
      UpdateData(FALSE);
      return;
	}
	str="进入子目录"+strDir;
	m_listdir.DeleteAllItems();  //删除视图控件中所有项
	CFtpFileFind finder(pFtpConnection);
	BOOL bWorking=finder.FindFile("*");  //查找所有文件
	while(bWorking)
	{
	  bWorking=finder.FindNextFile();   //查找下一个文件
	  if(finder.IsDirectory())   //如为子目录
		  strFlag=0;             //使用目录图像
	  else      //如为文件
		  strFlag=1;            //使用文件图像
	  //向列表视图控件中插入一个新项
	  m_listdir.InsertItem(Index++,finder.GetFileName(),strFlag);
	}
        finder.Close();
		m_state=str;
		UpdateData(FALSE);
	
}
