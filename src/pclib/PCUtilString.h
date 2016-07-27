#if !defined(_PCSTRINGUTIL_H)
#define _PCSTRINGUTIL_H
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


/**
*@brief		���ַ���ת��ΪСд
*@param		pszBuf		[IN]	��ת�����ַ��������øú�����pszBuf���ܱ��ı�
*@return	ָ��ת������ַ�����ָ��
*/
char* PCStrToLower(char *pszBuf);

/**
*@brief		���ַ���ת��Ϊ��д
*@param		pszBuf		[IN]	��ת�����ַ��������øú�����pszBuf���ܱ��ı�
*@return	ָ��ת������ַ�����ָ��
*/
char* PCStrToUpper(char *pszBuf);

/**
*@brief		�����ִ�Сд��strstr����
*@param		str1	[IN]	������Ŀ�꣬����ΪNULL��մ�
*@param		str2	[IN]	Ҫ���Ҷ��󣬲���ΪNULL��մ�
*@return	��str2��str1���Ӵ����򷵻�str2��str1���״γ��ֵĵ�ַ�����str2����str1���Ӵ����򷵻�NULL��
*/
const char* PCStrStricase(const char* str1, const char* str2);

/**
*@brief		���ַ���ȥ��ǰ/����ض��ַ���һ������ȥ�ո�
*@param		pszBuf		[IN]	��ת�����ַ�����ע�⣺1.���øú�����pszBuf���ܱ��ı� 2.���������һ���ܷ���ת������ַ���
*@param		pszWhat		[IN]	���Ƴ���ǰ/���ַ��б�����ָ������ַ�����" \n"���ʾ�Ƴ��ո����
*@param		nMode		[IN]	�Ƴ�ģʽ 1ȥ��� 2ȥ�ұ� 3ȥ��������
*@return	ָ��ת������ַ�����ָ��
*/
char*  PCStrTrim(char *pszBuf, const char *pszWhat = " ", int nMode = 3);

/**
*@brief		���ַ�������pszDelim�ָ��ӵ�vecStrList�У����vecStrList�Ѿ������ݣ���������ӣ�ԭ�������ݲ��ᱻɾ����
*@param		pszSrc		[IN]	���ָ���ַ���.ע�⣺1.���øú�����pszSrc���ܱ��ı�
*@param		pszDelim	[IN]	�ָ���ַ��б�����ָ������ַ�����" \n"���ʾ���տո���зָ�
*@param		vecStrList	[OUT]	�ָ����ַ����б�.ע�⣺���б��������ָ�룬���������������pszSrc
*@return	�����룬��PC_Lib.h
*/
int  PCStrSplit(char *pszSrc, const char *pszDelim, std::vector<char*>& vecStrList);

/**
*@brief		�������в��������������У����vecStrList�Ѿ������ݣ���������ӣ�ԭ�������ݲ��ᱻɾ����
*@param		argc		[IN]	�����в�������
*@param		argv    	[IN]	�����в����б�
*@param		vecStrList	[OUT]	�������ַ����б�
*@return	�����룬��PC_Lib.h
*/
int  PCCmdArgsParse(int argc, const char* const argv[], std::vector<std::string>& vecStrList);

/**
*@brief		��UTF8������ַ�������URL����
*			����RFC3986�涨��URL����ʱĬ��ʹ��UTF8ģʽ
*@param		pszSrc		[IN]	UTF8����Ĵ�URL�����ԭ��
*@param		pszDest		[OUT]	URL������ϵ�Ŀ�Ĵ��Ļ�����������URL�������ַ���
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@param		nMode		[IN]	URL�����ģʽ��
*								nMode = 1 ʹ��js1.5 encodeURI			82���������ַ�!#$&'()*+,/:;=?@-._~0-9a-zA-Z
*								nMode = 2 ʹ��js1.5 encodeURIComponent	71���������ַ�!'()*-._~0-9a-zA-Z
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCUrlEncode(const char *pszSrc, char *pszDest, unsigned int nDestBufLen, int nMode = 1);

/**
*@brief		��URL������ַ�������URL���룬�Ὣ+����Ϊ�ո񣬽������ַ�����������URL����ʱ���ַ�����
*@param		pszSrc		[IN]	��URL�����ԭ��
*@param		pszDest		[OUT]	URL������ϵ�Ŀ�Ĵ��Ļ�����
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCUrlDecode(const char *pszSrc, char *pszDest, unsigned int nDestBufLen);

/**
*@brief		��GBK������ַ���ת��ΪUTF8����
*@param		pszSrc		[IN]	GBK�����ԭ��
*@param		pszDest		[OUT]	ת��ΪĿ�Ĵ�UTF8�Ļ�����
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestת����ʵ�����ݵĳ��ȣ� <ʱΪ�����룬��PC_Lib.h
*/
int  PCGbkToUtf8(const char *pszSrc, char *pszDest, unsigned int nDestBufLen);

/**
*@brief		��UTF8������ַ���ת��ΪGBK����
*@param		pszSrc		[IN]	UTF8�����ԭ��
*@param		pszDest		[OUT]	ת��ΪĿ�Ĵ�GBK�Ļ�����
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCUtf8ToGbk(const char *pszSrc, char *pszDest, unsigned int nDestBufLen);


/**
*@brief		�������ֽ�������򣬽��������pszDest�С�pszSrc��pszDest�ĳ��������ȡ�
*@param		pszSrc		[IN]		��Ҫ�����ַ���1
*@param		pszDest		[IN/OUT]	��Ҫ�����ַ���2,����ԭ�����ַ���2����ʧ
*@param		nDestBufLen	[IN]		��Ҫ�����ֽ���
*@return	�����룬��PC_Lib.h
*/
int  PCBytesXor(const unsigned char *pszSrc, unsigned char *pszDest, unsigned int nXorLen);

/**
*@brief		����ʵ�ֽ�����ת��Ϊʮ��������ʾ���ַ�������"123"ת��Ϊ"313233"
*@param		pszSrc		[IN]	��ת������ʵ�ֽ����飬GBK����
*@param		nSrcLen		[IN]	��ת������ʵ�ֽ����鳤��
*@param		pszDestBuf	[OUT]	ת������ʾ�ַ����Ļ��������ⲿ�豣֤�����㹻
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@param		pszDelim	[IN]	ת����ÿ��ʮ�����Ʊ�ʾ���ֽ�֮��ķָ���(�8�ַ�)��Ĭ����
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestBufת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCBytes2DispHexStr(const unsigned char *pszSrc, unsigned int nSrcLen, char* pszDestBuf, unsigned int nDestBufLen, const char *pszDelim = "");

/**
*@brief		��ʮ��������ʾ���ַ���ת��Ϊ��ʵ�ֽ����飬��"31 32 33"ת��Ϊ"123"
*@param		pszSrc		[IN]	��ת����ʮ��������ʾ���ַ����������"0-9a-fa-A-F"Ϊ�ָ���
*@param		pszDestBuf	[OUT]	ת������ʾ�ַ����Ļ��������ⲿ�豣֤�����㹻�����ΪGBK����
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestBufת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCDispHexStr2Bytes(const  char *pszSrc, unsigned char* pszDestBuf, unsigned int nDestBufLen);

/**
*@brief		���ֽ��������ѹ����ʹ��GZIPģʽ 
*@param		pszSrc		[IN]	ԭ����δѹ��������
*@param		nSrcLen		[IN]	ԭ������
*@param		pszDestBuf	[OUT]	Ŀ�Ĵ����������ⲿ�豣֤�����㹻
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestBufת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCBytesGZipCompress(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen);

/**
*@brief		���ֽ�������н�ѹ����ʹ��GZIPģʽ
*@param		pszSrc		[IN]	ԭ�����Ѿ�ѹ���õ�����
*@param		nSrcLen		[IN]	ԭ������
*@param		pszDestBuf	[OUT]	Ŀ�Ĵ����������ⲿ�豣֤�����㹻
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestBufת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCBytesGZipDeCompress(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen);

/**
*@brief		��ȡһ��������޷���������������0
*@param		
*@return	�����������
*/
unsigned int  PCGetRandomUInt(void);

/**
*@brief		��һ�������������������ѡȡһ������������[3,7]��ѡ��6
*@param		nMin		[IN]	ѡȡ��������Сֵ����������
*@param		nMax		[IN]	ѡȡ���������ֵ����������
*@return	���ѡ����������.���nMin>=nMax���򷵻�nMin.
*/
unsigned int  PCGetRandomRange(unsigned int nMin, unsigned int nMax);

/**
*@brief		��ȡһ��������ֽ�����
*@param		pszDest		[OUT]	�ֽ����黺����
*@param		nDestBufLen	[IN]	��Ҫ�������ֽ�������
*@return	�����룬��PC_Lib.h
*/
int  PCGetRandomBytes(unsigned char *pszDest, unsigned int nDestLen);

/**
*@brief		���ֽ��������Base64����
*@param		pszSrc		[IN]	ԭ��
*@param		nSrcLen		[IN]	ԭ������
*@param		pszDestBuf	[OUT]	Ŀ�Ĵ����������ⲿ�豣֤�����㹻������ԭ�����������ռ��ǱȽϱ��յ�
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@param		bBlocked	[IN]	�Ƿ������Ƿֿ��(ÿ64�ֽ��û��зֿ�)��Ĭ�Ϸ�
*@param		bUrlSafe	[IN]	Python��url_safeģʽ�����ǽ���ͨbase64����Ľ�����ַ�+��/�ֱ���-��_ ��Ĭ�Ϸ�
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestBufת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCBase64Encode(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen, bool bBlocked = false, bool bUrlSafe = false);

/**
*@brief		��Base64���������ݽ��н���
*@param		pszSrc		[IN]	ԭ��
*@param		nSrcLen		[IN]	ԭ������
*@param		pszDestBuf	[OUT]	Ŀ�Ĵ����������ⲿ�豣֤�����㹻������ԭ�����������ռ��ǱȽϱ��յ�
*@param		nDestBufLen	[IN]	Ŀ�Ĵ�����������
*@param		bBlocked	[IN]	ԭ��������Ƿֿ��(ÿ64�ֽ��û��зֿ�)��Ĭ�Ϸ�
*@param		bUrlSafe	[IN]	Python��url_safeģʽ�����Ǳ������ǽ���ͨbase64����Ľ�����ַ�+��/�ֱ���-��_ ��Ĭ�Ϸ�
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszDestBufת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int  PCBase64Decode(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen, bool bBlocked = false, bool bUrlSafe = false);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCSTRINGUTIL_H)
