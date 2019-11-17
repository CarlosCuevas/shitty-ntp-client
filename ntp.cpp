#include <iostream>
#include <sys/socket.h> 
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ctime>
using namespace std;

typedef unsigned char byte_t;   // 8bit
typedef unsigned short word_t;  // 16bit
typedef unsigned int dword_t;   // 32bit
typedef unsigned long qword_t;  // 64bit

typedef struct {
    byte_t mode:3;      // mode
    byte_t version:3;   // version
    byte_t leap:2;      // leap second indicator
    byte_t stratum;     // how close are we to actual source
    byte_t poll;        // how often to query
    byte_t precision;   // precision
    dword_t rootdelay;  // roundtrip time
    dword_t rootdisp;   // root dispersion
    byte_t refid[4];    // reference id
    qword_t reftime;    // receive timestamp
    qword_t orig;       // origin timestamp
    qword_t rec;        // receive timestamp
    qword_t xmt;        // transmit timestamp
} ntp_packet;

int main(int argc, char *argv[]) {
    struct hostent *hostinfo;
    string ntp_domain = (argc > 1)? argv[1] : "pool.ntp.org";
    hostinfo = gethostbyname(ntp_domain.c_str());
    
    ntp_packet ntp_packet;
    memset(&ntp_packet, 0, sizeof(ntp_packet));
    ntp_packet.version = 1;

    struct sockaddr_in ntp_server;
    ntp_server.sin_family = AF_INET;
    ntp_server.sin_port = htons(123); // ntp port
    ntp_server.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list; // let's just use the first ip cause why not

    cout << "Using " << ntp_domain << " @ " << inet_ntoa(ntp_server.sin_addr) << endl << endl;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (!sock) {
        cout << "couldn't create socket" << endl;
        return 1;
    }
    
    int sent = sendto(sock, &ntp_packet, sizeof(ntp_packet), 0, (const struct sockaddr *) &ntp_server,  sizeof(ntp_server)); 
    if (!sent) {
        cout << "failed to send" << endl;
        return 1;
    }

    int len;
    int n = recvfrom(sock, (char *) &ntp_packet, 1024, MSG_WAITALL, (struct sockaddr *) &ntp_server, (socklen_t *) &len); 

    if (!n) {
        cout << "failed to receive" << endl;
        return 1;
    }

    // display
    char buffer[1024] = {0};
    ntp_packet.reftime = ntohl((time_t) ntp_packet.reftime) - 2208988800U;    
    ctime_r((time_t *)&ntp_packet.reftime, buffer);
    cout << "time when the system clock was last set: " << buffer << endl;
    
    ntp_packet.orig = ntohl((time_t) ntp_packet.orig) - 2208988800U;
    ctime_r((time_t *)&ntp_packet.orig, buffer);
    cout << "time at the client when the request departed: " << buffer << endl;
    
    ntp_packet.rec = ntohl((time_t) ntp_packet.rec) - 2208988800U;
    ctime_r((time_t *)&ntp_packet.rec, buffer);
    cout << "time at the server when the requested arrived: " << buffer << endl;
    
    ntp_packet.xmt = ntohl((time_t) ntp_packet.xmt) - 2208988800U;
    ctime_r((time_t *)&ntp_packet.xmt, buffer);
    cout << "time at the server when the request departed: " << buffer << endl;

    // display current system time (for reference)
    int system_time = time(NULL);
    ctime_r((time_t *)&system_time, buffer);
    cout << "system time: " << buffer << endl;
  
    close(sock);

    return 0;
}