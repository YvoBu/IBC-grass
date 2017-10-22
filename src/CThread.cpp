// **************************************************************************
//
// Modul-Name        : CThread.cpp
// Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
// Creation-Date     : 16.01.2011
// Modification-Date : 29.04.2011 07:08:56
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
// **************************************************************************
//                   E x t r a   I n c l u d e   L i s t
// **************************************************************************
#include <string>
#include <stddef.h>
#include <stdint.h>
#ifdef __linux__
#include <pthread.h>
#endif // __linux__
#include <memory.h>
// **************************************************************************
//                   F i r s t   I n c l u d e   L i s t
// **************************************************************************
#ifdef _WIN32
#endif // _WIN32
#ifdef __linux__
#include "CThread.h"
#endif // __linux__

int CThread::Count;
extern "C" {
// **************************************************************************
//
//  Method-Name       : startfnc()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 18.02.2011
//  Modification-Date : 30.04.2011 09:54:29
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void* startfnc(void* ptr){
    CThread *lpThread;
    void *return_code=0;
    
    lpThread=((CThread *)(ptr));
    if (lpThread->InitInstance()!=0) {
        return_code = (void*)(long int)(((CThread *)(ptr))->Run());
        CThread::Count--;
        lpThread->ExitInstance();
        delete lpThread;
        pthread_exit(return_code);
        return (return_code);
    } else {
        return (return_code);
    }
}
} //  extern C

// **************************************************************************
//
//  Method-Name       : CThread()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 16.01.2011
//  Modification-Date : 30.04.2011 09:45:55
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CThread::CThread() {
    ThreadID=0;
}
// **************************************************************************
//
//  Method-Name       : CThread()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 18.02.2011
//  Modification-Date : 30.04.2011 09:45:54
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CThread::CThread(const std::string& aName) {
    Name=aName;
    ThreadID=0;
}
// **************************************************************************
//
//  Method-Name       : ~CThread()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 18.02.2011
//  Modification-Date : 18.02.2011 13:53:26
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
CThread::~CThread() {
    
}
// **************************************************************************
//
//  Method-Name       : InitInstance()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 16.01.2011
//  Modification-Date : 18.02.2011 13:48:40
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
bool CThread::InitInstance() {
    return (true);
}
// **************************************************************************
//
//  Method-Name       : Create()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 18.02.2011
//  Modification-Date : 18.02.2011 14:14:40
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CThread::Create() {
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    CThread::Count++;
    pthread_create(&ThreadID, &attr, startfnc, (void *)(this));
    
}
// **************************************************************************
//
//  Method-Name       : Run()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 18.02.2011
//  Modification-Date : 18.02.2011 14:14:50
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
int CThread::Run() {
    return (-1);
}
// **************************************************************************
//
//  Method-Name       : ExitInstance()
//  Author            : Hans-Juergen Lange <hjl@simulated-universe.de>
//  Creation-Date     : 18.02.2011
//  Modification-Date : 18.02.2011 14:11:42
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CThread::ExitInstance() {
    
}
// **************************************************************************
//
//  Method-Name       : SetName()
//  Author            : 
//  Creation-Date     : 24.04.2011
//  Modification-Date : 24.04.2011 10:06:28
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
void CThread::SetName(const std::string& aName) {
    Name=aName;
}
// **************************************************************************
//
//  Method-Name       : GetName()
//  Author            : 
//  Creation-Date     : 24.04.2011
//  Modification-Date : 24.04.2011 10:07:11
//
//  Copyrights by Hans-Juergen Lange. All rights reserved.
//
// **************************************************************************
std::string& CThread::GetName() {
    return (Name);
}
