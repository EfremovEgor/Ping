#include <iostream>
#include "network_scanner.h"
#include <string>

using namespace std;

struct ICMPHeader
{
    unsigned char byType;
    unsigned char byCode;
    unsigned short nChecksum;
    unsigned short nId;
    unsigned short nSequence;
};
unsigned short CalculateChecksum(char *pBuffer, int length)
{
    unsigned short word_length;
    unsigned int sum_length = 0;

    for (int i = 0; i < length; i += 2)
    {
        word_length = ((pBuffer[i] << 8) & 0xFF00) + (pBuffer[i + 1] & 0xFF);
        sum_length += (unsigned int)word_length;
    }
    while (sum_length >> 16)
    {
        sum_length = (sum_length & 0xFFFF) + (sum_length >> 16);
    }
    sum_length = ~sum_length;
    return (unsigned short)sum_length;
}
int main()
{
    timeval timeInterval = {0, 0};
    timeInterval.tv_usec = 5000 * 1000;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        perror("Failed to create socket");
        exit(1);
    }

    ICMPHeader header;
    header.byCode = 0;
    header.nSequence = htons(1);
    header.byType = 8;
    header.nChecksum = 0;
    header.nId = htons(rand());

    int nMessageSize = 32;
    char *pSendBuffer = new char[sizeof(ICMPHeader) + nMessageSize];
    const char *ip1 = "173.194.221.101";
    const char *ip2 = "192.168.0.21";
    char const *destination_ip = ip2;

    sockaddr_in destination;
    destination.sin_addr.s_addr = inet_addr(destination_ip);
    destination.sin_family = AF_INET;
    destination.sin_port = rand();

    memcpy(pSendBuffer, &header, sizeof(ICMPHeader));
    memset(pSendBuffer + sizeof(ICMPHeader), 'x', nMessageSize);

    header.nChecksum = htons(CalculateChecksum(pSendBuffer, sizeof(ICMPHeader) + nMessageSize));

    memcpy(pSendBuffer, &header, sizeof(ICMPHeader));

    int send_result = sendto(sock, pSendBuffer, sizeof(ICMPHeader) + nMessageSize, 0, (sockaddr *)&destination, sizeof(sockaddr_in));

    if (send_result == -1)
    {
        cout << "Error occured in sendto operation";
        delete[] pSendBuffer;
        exit(1);
    }
    fd_set fdRead;
    FD_ZERO(&fdRead);
    FD_SET(sock, &fdRead);
    send_result = select(sock + 1, &fdRead, NULL, NULL, &timeInterval);

    if (send_result == -1)
    {
        cout << "Error occured in select operation";
        exit(1);
    }

    if (!(send_result > 0 && FD_ISSET(sock, &fdRead)))
    {
        cout << "No packet recieved back\n";
        exit(1);
    }
    char *pRecieveBuffer = new char[1500];
    int receive_result = recvfrom(sock, pRecieveBuffer, 1500, 0, 0, 0);
    cout << receive_result;
    exit(1);
}
