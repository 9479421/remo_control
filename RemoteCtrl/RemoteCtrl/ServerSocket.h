#pragma once



#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:



	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {

	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0XFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0XFEFF)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) //�����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ�
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) //��δ��ȫ���յ����ͷ��أ�����ʧ��
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i; // head2 length4 data
			return;
		}
		nSize = 0;
	}
	~CPacket() {

	}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() { //�����ݵĴ�С
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)(pData) = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}


public:
	WORD sHead; //�̶�λFE FF
	DWORD nLength; //�����ȣ��ӿ������ʼ������У�飩
	WORD sCmd; //��������
	std::string strData; //������
	WORD sSum; //��У��
	std::string strOut;
};
#pragma pack(pop)

typedef	struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction; //������ƶ���˫��
	WORD nButton; //������Ҽ����м�
	POINT ptXY; //����
}MOUSEEV, * PMOUSEEV;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid; //�Ƿ���Ч
	BOOL IsDirectory;  //�Ƿ�ΪĿ¼ 0 �� 1 ��
	BOOL HasNext; //�Ƿ��к��� 0 û�� 1 ��
	char szFileName[256]; //�ļ���
}FILEINFO, * PFILEINFO;


class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL)
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket() {
		if (m_sock == -1)
		{
			return false;
		}

		//SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, 0);
		//if (serv_sock == -1) return false;
		SOCKADDR_IN serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(9527);
		serv_addr.sin_addr.S_un.S_addr = INADDR_ANY;
		if (bind(m_sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == -1)
		{
			return false;
		}
		if (listen(m_sock, 1) == -1)
		{
			return false;
		}
		return true;
	}

	bool AcceptClient() {
		SOCKADDR_IN cli_addr;
		int cli_sz = sizeof(cli_addr);
		m_client = accept(m_sock, (sockaddr*)&cli_addr, &cli_sz);
		TRACE("AcceptClient %d\n", m_client);
		if (m_client == -1)
		{
			return false;
		}
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client == -1)
		{
			return -1;
		}
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL)
		{
			TRACE("�ڴ治��\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;

		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				delete[]buffer;
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}

	bool Send(const char* pData, size_t nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0);
	}
	bool Send(CPacket& pack) {
		if (m_client == -1) return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if (m_packet.sCmd == 2 ||
			m_packet.sCmd == 3 ||
			m_packet.sCmd == 4 ||
			m_packet.sCmd == 9)
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseClient() {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
private:
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
	CServerSocket& operator= (const CServerSocket& ss) {

	}
	CServerSocket(const CServerSocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�����׽��ֳ�ʼ��ʧ��"), _T("��ʾ"), MB_OK | MB_ICONERROR);;
			exit(0);
		}
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}
	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL)
		{
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CServerSocket* m_instance;

	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_herper;
};
