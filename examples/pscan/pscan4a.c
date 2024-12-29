#include "neco.h"
#include <stdio.h>
#include <unistd.h>

void port_scan(int argc, void** argv) {
    (void)argc;
    int i = *(int*)argv[0];
    neco_waitgroup* wg = (neco_waitgroup*)argv[1];
    // 
    char host[32];
    sprintf(host, "scanme.nmap.org:%d", i);

    int fd = neco_dial_dl("tcp", host, neco_now() + NECO_SECOND);
    if (fd < 0) {
        printf("Port %d closed\n", i);
    }
    else {
        printf("Connected to port %d\n", i);
        close(fd);
    }
    neco_waitgroup_done(wg);
}

int neco_main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // synchronized counter - waitgroup
    neco_waitgroup wg;
    neco_waitgroup_init(&wg);
    
    int ports[] = {22, 80, 8080, 443};

    for (int i = 0; i < 4; i++) {
        //        
        neco_waitgroup_add(&wg, 1);
        neco_start(port_scan, 2, &ports[i], &wg);
    } 
    
    neco_waitgroup_wait(&wg);
    return 0;
}
