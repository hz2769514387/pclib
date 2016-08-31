

#include "pclib/PC_Lib.h"
#include "pclib/PCLog.h"
#include "pclib/PCTimeValue.h"
#include "pclib/PCConfig.h"
#include "pclib/PCThread.h"
#include "pclib/PCService_Win.h"
#include "pclib/PCBlockingQueue.h"
#include "pclib/PCBuffer.h"
#include "pclib/PCDisruptor.h"


using namespace std;
using namespace pclib;

class THR : public CPCThread
{
public:
	THR() {}

public:
	void init(int  name)
	{
		sprintf(szThreadName, "%d",name);
		
		PC_INFO_LOG("I AM INIT ! -- [%s]", szThreadName);

		
	}
    void Svc() override
	{
		while (m_bRunning)
		{
			//PC_INFO_LOG( "I AM RUNNING ! -- [%s]", szThreadName);
			PCSleepMsec(50);
		}
		PC_INFO_LOG("I AM over ! -- [%s]", szThreadName);
	}

	
protected:
    mutable char szThreadName[10];
};





int main(int argc, char* argv[])
{
	CPCLog::GetRoot()->SetLogAttr(CPCLog::eLevelTrace, CPCLog::eGenModeDay, true, "./logs");

	//TIME
	char pszTimeS[50];
	CPCTimeValue tv = CPCTimeValue::Now();
	tv.Format("Sleep Start: %Y-%m-%d %H:%M:%S @@@", pszTimeS, 50);
	PC_INFO_LOG( "%s", pszTimeS);

	while (true)
	{
		if (tv.IsTimeOut(00))
		{
			break;
		}
	}
	tv = CPCTimeValue::Now();
	tv.Format("Sleep 1000ms End: %Y-%m-%d %H:%M:%S @@@", pszTimeS, 50);
	PC_INFO_LOG( "%s", pszTimeS);

	CPCTimeValue tv3;
	tv3 = "2016-06-24 14:10:51";
	tv3.Format("tv3 set: %Y-%m-%d %H:%M:%S @@@", pszTimeS, 50);
	PC_INFO_LOG( "%s", pszTimeS);

	tv3 -= 1000;
	tv3.Format("tv3-=1000: %Y-%m-%d %H:%M:%S @@@", pszTimeS, 50);
	PC_INFO_LOG( "%s", pszTimeS);

	CPCTimeValue tv5 = CPCTimeValue::TickCount();
	tv5.Format("tv5 tick: %Y-%m-%d %H:%M:%S @@@", pszTimeS, 50);
	PC_INFO_LOG( "%s", pszTimeS);

    CPCConfig ffg1;
    std::map<string,string> ffm;
    int ncfg = ffg1.GetSection("Common",ffm,"/home/hz/config.ini");

    {
        std::shared_ptr<int> autof(new int);
		std::weak_ptr<int> ts = autof;
		std::shared_ptr<int> hign = ts.lock();

    }
	

	//PCBuffer
	CPCBuffer buf(8);
	buf.Append("hello,world", 11);
	buf.Append("fff59988", 8);
	const char * pszzz = buf.C_Str();
	buf.Reset(4);
	pszzz = buf.C_Str();

    CPCBuffer bfu("hello,world");
	

	//BLOCKINGQUEUE
	CPCBlockingQueue<int> queue;
	queue.Put(44);
    int cc = queue.Front();
    int aa = queue.Take();

	//RANDOM
	unsigned char pszRand[10];
	for (int i = 0; i < 1; i++)
	{
		PCGetRandomBytes(pszRand, 10);
	
		PC_INFO_LOG("随机数：%u", PCGetRandomRange(3, 20));
		CPCLog::GetRoot()->WriteLogBytes(__FUNCTION__, __LINE__, CPCLog::eLevelWarn, "随机数：", pszRand, 10);
	}
	

   
	unsigned char szout[1111] = {0};
	PCDispHexStr2Bytes("40313931202030313536353139363133313238313237333630352030313032303530303138363737373734323538202020202020202020313132332020203132332020202020202020202020202020202020202020202020202020202020303030303131303030303031",
		szout, 1111);

	std::vector<std::string> vecStrList;
	int reta = PCCmdArgsParse(argc, argv, vecStrList);

	unsigned char pszDestBufbb[1111];
	int fff = PCBase64Encode((unsigned char*)"i\xb7\x1d\xfb\xef\xff", 6, pszDestBufbb, 1111, false, true);

	unsigned char pszSSS[1111];
	int ggg = PCBase64Decode((unsigned char*)"YWJjZA", 6, pszSSS, 1111, false, false);
	
	unsigned char pszDest3[1001] = { 0 };
	const  char *pszSrc1 = "123";
	const  char *pszSrc2 = "12345678";
	const  char *pszSrc3 = "1234567812345678";
	const  char *pszSrc4 = "12345678123456781";
	const  char *pszSrc5 = "123456781234567";
	//8b 9d c9 c5 7a 7e c9 27
	int LL = PCSymEncypt(PC_3DES_ECB, (unsigned char*)pszSrc1, strlen(pszSrc1), (unsigned char *)"12345678", 8, pszDest3, sizeof(pszDest3), "pboc", NULL);
	unsigned char pszDest13[1001] = { 0 };
	//
	LL = PCSymDecypt(PC_AES_ECB, pszDest3, LL, (unsigned char *)"1234567812345678", 16, pszDest13, sizeof(pszDest13), "pboc", NULL);



	//////////////////////////
	THR foo[100];
	for (int i = 0; i < 100; i++)
	{
		foo[i].init(i);
		foo[i].StartThread();
	}
	PCSleepMsec(100);

	for (int i = 0; i < 100; i++)
	{
		foo[i].StopThread();
		
	}
	PCSleepMsec(100);
	

	


	unsigned char pszDest[122] = { 0 };
	const char * psrc = "你好世界。         <p>尊敬的用户，您好！</p><p>抱歉，服务器出小差了，请稍后再试。< / p>";
	unsigned int psrcLen = strlen(psrc);

	const char * pkey = "你fidsjfjedopisfejoifoewf898432iojrferskofjkdlsjfkldsjfklds好世界。         <p>尊敬的用户，您好！</p><p>抱歉，服务器出小差了，请稍后再试。< / p>";
	unsigned int pkeyLen = strlen(pkey);
	/*a0 f1 a0 14 73 56 12 c6   91 cc 59 ae cb 7b dc db*/
	int NRET1 = PCGetMd(PC_ALGO_MD5,(unsigned char*)psrc, psrcLen, pszDest);

	/* 93 e1 54 86 4b 24 04 4a   b1 f0 cc b8 ff cc 4c a6*/
	int NRET2 = PCGetFileMd(PC_ALGO_MD5,"d:\\1.1", pszDest);

	int NRET = PCGetHMac(PC_ALGO_MD5, (unsigned char*)psrc, psrcLen, (unsigned char*)pkey, pkeyLen, pszDest);
	
	CPCLog::GetRoot()->WriteLogBytes(__FUNCTION__, __LINE__, CPCLog::eLevelTrace, "MD5：", pszDest, 16);
	

	unsigned char pszDest12[1001] = { 0 };

	CPCConfig ffg;
	int nCfgLine = ffg.GetCfg("DBPool1", "DBUser", (char*)pszDest12, 1001, "config.ini");
	nCfgLine = ffg.SetCfg("DBPool1", "DBUser", "122", "config.ini");

	
	unsigned long ulCOA32;
	int FF = PCGetFileCOA32("d:\\1.1", ulCOA32);
	PC_INFO_LOG( "CRC：%x", ulCOA32);

	

	
	const char * in = "你好世界。         <p>尊敬的用户，您好！</p><p>抱歉，服务器出小差了，请稍后再试。< / p>";
	unsigned char pszDestBuf[122] = { 0 };
	int ret = PCBytesGZipCompress((unsigned char*)in, strlen(in), pszDestBuf, 122);
	PC_INFO_LOG( "压缩后：%d字节", ret);
	CPCLog::GetRoot()->WriteLogBytes(__FUNCTION__, __LINE__, CPCLog::eLevelTrace, "数据：", pszDestBuf, ret);

	unsigned char pszDestBuf2[122] = { 0 };
	ret = PCBytesGZipDeCompress(pszDestBuf, ret, pszDestBuf2, 122);
	PC_INFO_LOG( "解压缩后：%d字节， 数据：%s", ret, pszDestBuf2);

	PCSleepMsec(1000);

	
}
