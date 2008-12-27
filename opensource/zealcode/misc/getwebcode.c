/*
 * get a  web's source code function.
 *
 * usage: *.ext www.hao123.com > code.txt
 *
 * copy from cu.bbs
 *
 *
 *
 *
 * */
#include        <stdio.h>
#include        <winsock.h>
#include        <string.h>
#pragma comment(lib, "ws2_32.lib")

static void wsa_startup(void)
{
#ifdef WIN32
	static int wsa_started = 0;
	WSADATA data;
	
	if (!wsa_started) {
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
			return;
		wsa_started = 1;
	}
#endif
}

static void wsa_cleanup(void)
{
#ifdef WIN32
	WSACleanup();
#endif
}

void geturl(char *url)
{
        SOCKET        sockfd;
        struct sockaddr_in        addr;
        struct hostent        *pURL;
        char        myurl[BUFSIZ];
        char        *pHost = 0, *pGET = 0;
        char        host[BUFSIZ], GET[BUFSIZ];
        char        header[BUFSIZ] = "";
        static char        text[BUFSIZ];
        int i;
        
        wsa_startup();
        /*
         *        ����url�е�������ַ�����·��
         */
        strcpy(myurl, url);
        for (pHost = myurl; *pHost != '/' && *pHost != '\0'; ++pHost);
        if ( (int)(pHost - myurl) == strlen(myurl) )
                strcpy(GET, "/");
        else
                strcpy(GET, pHost);
        *pHost = '\0';
        strcpy(host, myurl);
        printf("%s\n%s\n", host, GET);

        /*
         *        �趨socket����,��δ������ʼ��
         */
        sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        pURL = gethostbyname(host);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = *((unsigned long*)pURL->h_addr);
        addr.sin_port = htons(80);

        /*
         *        ��֯���͵�web����������Ϣ
         *        Ϊ��Ҫ�����������Ϣ��ο�HTTPЭ���Լ��
         */
        strcat(header, "GET ");
        strcat(header, GET);
        strcat(header, " HTTP/1.1\r\n");
        strcat(header, "HOST: ");
        strcat(header, host);
        strcat(header, "\r\nConnection: Close\r\n\r\n");
        
        /*
         *        ���ӵ�����������������header�������ܷ���������ҳԴ���룩
         */
        connect(sockfd,(SOCKADDR *)&addr,sizeof(addr));
        
        send(sockfd, header, strlen(header), 0);
        
        while ( recv(sockfd, text, BUFSIZ, 0) > 0)
        {        
                printf("%s", text);
                strnset(text, '\0', BUFSIZ);
        }

        closesocket(sockfd);
        
        wsa_cleanup();
}

int main()
{
        char        url[256];
        printf("http://");
        scanf("%s", url);
        geturl(url);
        return 0;
}
