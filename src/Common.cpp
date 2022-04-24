#include "Common.h"
#include "Windows.h"

//��stringת����wstring
std::wstring string2wstring(std::string str)
{
	std::wstring result;
	//��ȡ��������С��������ռ䣬��������С���ַ�����
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//���ֽڱ���ת���ɿ��ֽڱ���
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //����ַ�����β
	//ɾ��������������ֵ
	result.append(buffer);
	delete[] buffer;
	return result;
}

