#include <string>
#include <sstream>
#include <sys/select.h>
#include "beginvision.h"

#include "libnatpmp/natpmp.h"

#define  JNIDEFINE(fname) Java_teaonly_droideye_MainActivity_##fname

extern "C" {
    JNIEXPORT jstring JNICALL JNIDEFINE(nativeQueryInternet)(JNIEnv* env, jclass clz);
};


jstring JNICALL JNIDEFINE(nativeQueryInternet)(JNIEnv* env, jclass clz) {
    natpmp_t natpmp;
	natpmpresp_t response;
    std::string ipaddr; 
    uint16_t privatePort = 8080;
    uint16_t publicPort = 7910;
    uint32_t lifetime = 36000;              // 10 hours
    int protocol = NATPMP_PROTOCOL_TCP;
    in_addr_t gateway = 0;
	fd_set fds;
	struct timeval timeout;
    int ret;
    std::string  result = "";
        
    // 1. init internal object
    ret = initnatpmp(&natpmp, 0, gateway);
    if (ret < 0) {
        result = "error:gateway";
        goto _done;
    }

    // 2. get internet public ip address
    ret = sendpublicaddressrequest(&natpmp);
    if ( ret != 2) {
        result = "error:ipaddr";
        goto _done;
    } 
    FD_ZERO(&fds);
    FD_SET(natpmp.s, &fds);
    getnatpmprequesttimeout(&natpmp, &timeout);
    ret = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
    if(ret < 0) {
        result = "error:select";
        goto _done;
    }
    ret = readnatpmpresponseorretry(&natpmp, &response);
    if( ret == NATPMP_TRYAGAIN  ) {
        result = "error:timeout";
        goto _done;
    } else if ( ret < 0) {
        result = "error:failed";
        goto _done;
    } 
    ipaddr = inet_ntoa(response.pnu.publicaddress.addr);
    
    // 3. set extern port mapping
    ret = sendnewportmappingrequest(&natpmp, protocol,
        	                      privatePort, publicPort,
								  lifetime);
    FD_ZERO(&fds);
    FD_SET(natpmp.s, &fds);
    getnatpmprequesttimeout(&natpmp, &timeout);
    ret = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
    if(ret < 0) {
        result = "error:select";
        goto _done;
    }
    ret = readnatpmpresponseorretry(&natpmp, &response);
    if( ret == NATPMP_TRYAGAIN  ) {
        result = "error:timeout";
        goto _done;
    } else if ( ret < 0) {
        result = "error:failed";
        goto _done;
    } 
    privatePort = response.pnu.newportmapping.privateport;
    publicPort = response.pnu.newportmapping.mappedpublicport; 
   
    {
        std::stringstream stream;
        stream << "http://" << ipaddr << ":" << publicPort;
        result = stream.str();
    }

_done:
    return env->NewStringUTF(result.c_str());  
}

