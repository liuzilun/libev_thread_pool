#include "XFtpFactory.h"
#include "XFtpServerCMD.h"
#include"XFtpUSER.h"
#include"XFtpLIST.h"
#include"XFtpPORT.h"
#include"XFtpRETR.h"
#include"XFtpSTOR.h"


XTask* XFtpFactory::CreateTask()
{
	XFtpServerCMD* x = new XFtpServerCMD();

	//ע��ftp��Ϣ�������
	x->Reg("USER", new XFtpUSER());
	
	XFtpLIST* list = new XFtpLIST();
	
	x->Reg("RETR", new XFtpRETR());
    x->Reg("PWD", list);
	x->Reg("CWD", list);
	x->Reg("CDUP", list);
	x->Reg("PORT", new XFtpPORT());
	x->Reg("STOR", new XFtpSTOR());
	x->Reg("LIST", list);



	return x;
}


XFtpFactory::XFtpFactory()
{
}


