#include "StdAfx.h"
#include "ThreadHelper.h"

ThreadHelper::ThreadHelper(void)
{
    m_dwThreadID        = 0;
    m_hThreadHandle     = INVALID_HANDLE_VALUE;
    m_hStopEvent        = INVALID_HANDLE_VALUE;
    m_pThreadDelegate   = NULL;
    m_bExit             = FALSE;
}

ThreadHelper::~ThreadHelper(void)
{
    Stop();
    CloseHandle(m_hStopEvent);
    CloseHandle(m_hThreadHandle);

}

bool ThreadHelper::Create(ThreadHelperDelegate* pDelegate, bool bStart /*= true*/ )
{
    if(pDelegate != NULL && m_hThreadHandle != INVALID_HANDLE_VALUE) return true;

    m_pThreadDelegate = pDelegate;
    m_hThreadHandle = (HANDLE)::_beginthreadex(NULL,0,ThreadFun,this,bStart ? 0 : CREATE_SUSPENDED ,NULL);
    
    if(m_hThreadHandle != 0)
    {
        m_hStopEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
        return true;
    }
    m_hThreadHandle = INVALID_HANDLE_VALUE;
    m_pThreadDelegate = NULL;
    return false;
}

bool ThreadHelper::Start()
{
    if(m_hThreadHandle == INVALID_HANDLE_VALUE && m_bExit == TRUE)  return false;

    DWORD dwRet = ::ResumeThread(m_hThreadHandle);
    return dwRet != -1 ? true : false;
}

void ThreadHelper::Stop( int time /*= INFINITE*/ )
{
    if(InterlockedCompareExchange(&m_bExit,TRUE,TRUE))  return;

    InterlockedExchange(&m_bExit,TRUE);
   ::WaitForSingleObject(m_hStopEvent,time);
}

unsigned __stdcall ThreadHelper::ThreadFun( void* lpParam )
{
    ThreadHelper* pThis = (ThreadHelper*)lpParam;
    if(NULL != pThis->m_pThreadDelegate)
    {
         pThis->m_pThreadDelegate->Run();
    }
   

    InterlockedExchange(&pThis->m_bExit,TRUE);
    SetEvent(pThis->m_hStopEvent);
    return 0;
}





//////////////////////////////////////////ThReadLoopHelper////////////////////////
ThreadLoopHelper::ThreadLoopHelper()
{
    m_bInit = false;
    m_bLoop = TRUE;
    InitializeCriticalSection(&m_Section);
}

ThreadLoopHelper::~ThreadLoopHelper()
{
    DeleteCriticalSection(&m_Section);
}

bool ThreadLoopHelper::Init()
{   
    m_ThreadHelper.Create(this);
    m_hPostEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
    return true;
}

void ThreadLoopHelper::UnInit()
{
    //helper 是一个成员变量的局部变量，自己会最先析构，会先析构释放
    InterlockedExchange(&m_bLoop,FALSE);
    m_bInit = false;
    SetEvent(m_hPostEvent);
    CloseHandle(m_hPostEvent);
    m_ThreadHelper.Stop();
}

bool ThreadLoopHelper::PostEvent(ThreadLoopHelper::Event::fun f,void* lpParam)
{
    //同步
    Event*  pEv = new Event(f,lpParam);
    EnterCriticalSection(&m_Section);
    m_EventList.push_back(pEv);
    LeaveCriticalSection(&m_Section);
    SetEvent(m_hPostEvent);
    return true;
}

bool ThreadLoopHelper::PostEventToHead(ThreadLoopHelper::Event::fun f,void* lpParam)
{
    //同步
    Event*  pEv = new Event(f,lpParam);
    EnterCriticalSection(&m_Section);
    m_EventList.push_front(pEv);
    LeaveCriticalSection(&m_Section);
    SetEvent(m_hPostEvent);
    return true;
}

//在别的线程
void ThreadLoopHelper::Run()
{
    EventList eventList;
    while (InterlockedCompareExchange(&m_bLoop, TRUE, TRUE))
    {
        ::WaitForSingleObject(m_hPostEvent, INFINITE);
        EnterCriticalSection(&m_Section);
        if (m_EventList.size() > 0)
        {
            eventList.swap(m_EventList);
        }
        LeaveCriticalSection(&m_Section);

        for (EventList::iterator it = eventList.begin(); it != eventList.end(); ++it)
        {
            Event *pEvent = *it;
            pEvent->DoIt();
            delete pEvent;
        }
        eventList.clear();
    }
    InterlockedExchange(&m_bLoop, FALSE);
}

ThreadLoopHelper& ThreadLoopHelper::GetInstance()
{
    static ThreadLoopHelper instance;
    return instance;
}


////////////////////////////Event
ThreadLoopHelper::Event::Event( fun f,void* lpParam)
{
    m_fEvent = f;
    m_pParam = lpParam;
}

ThreadLoopHelper::Event::~Event()
{
    //自己管理释放 传入参数
}

void ThreadLoopHelper::Event::DoIt()
{
    if(NULL != m_fEvent)
    {
        m_fEvent(m_pParam);
    }
}