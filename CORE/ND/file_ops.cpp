#include "types.h"
#include "rillog.h"
#include "file_ops.h"
#include <fcntl.h>
#include <cutils/sockets.h>
#include <termios.h>
#include <sys/stat.h>
#include <time.h>

// Use to convert our timeout to a timeval in the future
timeval msFromNowToTimeval(UINT32 msInFuture)
{
    timeval FutureTime;
    timeval Now;
    UINT32 usFromNow;

    gettimeofday(&Now, NULL);

    usFromNow = ((msInFuture %  1000) * 1000) + Now.tv_usec;

    FutureTime.tv_sec = Now.tv_sec + (msInFuture / 1000);

    if (usFromNow >= 1000000)
    {
        FutureTime.tv_sec++;
        FutureTime.tv_usec = (usFromNow - 1000000);
    }
    else
    {
        FutureTime.tv_usec = usFromNow;
    }

    return FutureTime;
}

CFile::CFile() :
    m_file(-1),
    m_fInitialized(FALSE) {};

CFile::~CFile()
{
    if (m_fInitialized)
    {
        if (0 > close(m_file))
        {
            RIL_LOG_CRITICAL("CFile::~CFile() : ERROR : Error when closing file\r\n");
        }

        m_fInitialized = FALSE;
    }
}

BOOL CFile::OpenSocket(const char * pszSocketName)
{
    if (NULL == pszSocketName)
    {
        RIL_LOG_CRITICAL("CFile::OpenSocket() : ERROR : pszSocketName was NULL\r\n");
        goto Error;
    }

    m_file = socket_local_client(pszSocketName, ANDROID_SOCKET_NAMESPACE_FILESYSTEM, SOCK_STREAM);

    if (m_file < 0)
    {
        RIL_LOG_CRITICAL("CFile::OpenSocket() : ERROR : Could not open socket\r\n");
        goto Error;
    }

    m_fInitialized = TRUE;

Error:
   return m_fInitialized;
}

BOOL CFile::Open(   const char * pszFileName,
                    UINT32 dwAccessFlags,
                    UINT32 dwOpenFlags,
                    UINT32 dwOptFlags,
                    BOOL fIsSocket)
{
    RIL_LOG_INFO("CFile::Open() - Enter\r\n");
    RIL_LOG_INFO("CFile::Open() : pszFileName=[%s]\r\n", (NULL == pszFileName ? "NULL" : pszFileName));
    RIL_LOG_INFO("CFile::Open() : fIsSocket=[%d]\r\n", fIsSocket);

    UINT32 dwAttr = 0;
    BOOL fExists = FALSE;
    BOOL fFile = FALSE;
    int iAttr = 0;

    if (m_fInitialized)
    {
        RIL_LOG_CRITICAL("CFile::Open() : ERROR : File is already open. Close first.\r\n");
        goto Error;
    }

    if (fIsSocket)
    {
        OpenSocket(pszFileName);
    }
    else
    {
        dwAttr = GetFileAttributes(pszFileName);

        fExists = (dwAttr == FILE_ATTRIB_DOESNT_EXIST) ? FALSE : TRUE;
        fFile   = (dwAttr == FILE_ATTRIB_DOESNT_EXIST) ? FALSE : ((dwAttr & FILE_ATTRIB_REG) ? TRUE : FALSE);
        RIL_LOG_INFO("CFile::Open() : fExists=[%d]  fFile=[%d]\r\n", fExists, fFile);

        switch (dwAccessFlags)
        {
            case FILE_ACCESS_READ_ONLY:
            {
                iAttr = O_RDONLY;
                break;
            }

            case FILE_ACCESS_WRITE_ONLY:
            {
                iAttr = O_WRONLY;
                break;
            }

            case FILE_ACCESS_READ_WRITE:
            {
                RIL_LOG_INFO("--- FILE_ACCESS_READ_WRITE --- Pranav\r\n ");
                //iAttr = O_RDWR;
                iAttr = CLOCAL | O_NONBLOCK | O_RDWR;       // Pranav
                break;
            }

            default:
            {
                RIL_LOG_CRITICAL("CFile::Open() : ERROR : Invalid access flags given: 0x%X\r\n", dwAccessFlags);
                goto Error;
            }
        }

        // Only open if file already exists
        if (FILE_OPEN_EXIST & dwOpenFlags)
        {
            if (!fExists)
            {
                RIL_LOG_CRITICAL("CFile::Open() : ERROR : File does not already exist, unable to open\r\n");
                goto Error;
            }
        }

        if (FILE_OPEN_NON_BLOCK & dwOpenFlags)
        {
            iAttr |= O_NONBLOCK;
        }

        if (FILE_OPEN_APPEND & dwOpenFlags)
        {
            iAttr |= O_APPEND;
        }

        if (FILE_OPEN_CREATE & dwOpenFlags)
        {
            iAttr |= O_CREAT;
        }

        if (FILE_OPEN_TRUNCATE & dwOpenFlags)
        {
            iAttr |= O_TRUNC;
        }

        if (FILE_OPEN_EXCL & dwOpenFlags)
        {
            iAttr |= O_EXCL;
        }

        m_file = open(pszFileName, iAttr | CLOCAL | O_NONBLOCK);        // Pranav, check if we need these changes

        if (m_file < 0)
        {
            RIL_LOG_CRITICAL("CFile::Open() : ERROR : open failed, m_file=[0x%08x], [%d]\r\n", m_file, m_file);
            RIL_LOG_CRITICAL("CFile::Open() : ERROR : errno=[%d],[%s]\r\n", errno, strerror(errno));
            goto Error;
        }
        else
        {
            RIL_LOG_CRITICAL("**********CFile::Open() : m_file=[%d]\r\n", m_file);
        }

        {
            termios terminalParameters;
            int err = tcgetattr(m_file, &terminalParameters);
            if (-1 != err)
            {
                cfmakeraw(&terminalParameters);
                tcsetattr(m_file, TCSANOW, &terminalParameters);
            }
        }

        m_fInitialized = TRUE;
    }

Error:
    RIL_LOG_INFO("CFile::Open() - Exit  m_fInitialized=[%d]\r\n", m_fInitialized);
    return m_fInitialized;
}

BOOL CFile::Close()
{
    if (!m_fInitialized)
    {
        RIL_LOG_CRITICAL("CFile::Close() : ERROR : File is not open! Unable to close\r\n");
        return FALSE;
    }

    RIL_LOG_CRITICAL("**********CFile::Close() - Closing %d \r\n", m_file);
    if (0 > close(m_file))
    {
        RIL_LOG_CRITICAL("CFile::Close() : ERROR : Error when closing file\r\n");
        return FALSE;
    }

    m_file = -1;
    m_fInitialized = FALSE;
    return TRUE;
}

BOOL CFile::Write(const void * pBuffer, UINT32 dwBytesToWrite, UINT32 &rdwBytesWritten)
{
    int iCount = 0;
    rdwBytesWritten = 0;

    if (!m_fInitialized)
    {
        RIL_LOG_CRITICAL("CFile::Write() : ERROR : File is not open!\r\n");
        return FALSE;
    }

    if ((iCount = (write(m_file, pBuffer, dwBytesToWrite))) == -1)
    {
        RIL_LOG_CRITICAL("CFile::Write() : ERROR : Error during write process!\r\n");
        return FALSE;
    }

    rdwBytesWritten = (UINT32)iCount;

    return TRUE;
}

BOOL CFile::Read(void * pBuffer, UINT32 dwBytesToRead, UINT32 &rdwBytesRead)
{
    int iCount = 0;
    rdwBytesRead = 0;

    if (!m_fInitialized)
    {
        RIL_LOG_CRITICAL("CFile::Read() : ERROR : File is not open!\r\n");
        return FALSE;
    }

    if ((iCount = read(m_file, pBuffer, dwBytesToRead)) == -1)
    {
        if (errno != EAGAIN)
        {
            RIL_LOG_CRITICAL("CFile::Write() : ERROR : Error during read process!\r\n");
            return FALSE;
        }

        rdwBytesRead = 0;
        return TRUE;
    }

    rdwBytesRead = (UINT32)iCount;

    return TRUE;
}

UINT32 CFile::GetFileAttributes(const char * pszFileName)
{
    struct stat statbuf;
    UINT32 dwReturn;

    if (stat(pszFileName, &statbuf))
        return FILE_ATTRIB_DOESNT_EXIST;

    dwReturn = 0;

    if (S_ISDIR(statbuf.st_mode))
    {
        dwReturn |= FILE_ATTRIB_DIR;
    }

    if (S_ISREG(statbuf.st_mode))
    {
        dwReturn |= FILE_ATTRIB_REG;
    }

    if (S_ISSOCK(statbuf.st_mode))
    {
        dwReturn |= FILE_ATTRIB_SOCK;
    }

    if ((statbuf.st_mode & S_IWUSR) == 0)
    {
        dwReturn |= FILE_ATTRIB_RO;
    }

    if (S_ISBLK(statbuf.st_mode))
    {
        dwReturn |= FILE_ATTRIB_BLK;
    }

    if (S_ISCHR(statbuf.st_mode))
    {
        dwReturn |= FILE_ATTRIB_CHR;
    }

    if (!dwReturn)
    {
        dwReturn = FILE_ATTRIB_REG;
    }

    return dwReturn;
}

BOOL CFile::WaitForEvent(UINT32 &rdwFlags, UINT32 dwTimeoutInMS)
{
    fd_set readfs, writefs, errorfs;
    int maxfd = m_file + 1;

    if (m_file < 0)
    {
        RIL_LOG_CRITICAL("CFile::WaitCommEvent() : ERROR : m_file was not valid");
        return FALSE;
    }

    FD_ZERO(&readfs);
    FD_ZERO(&errorfs);

    rdwFlags = 0;

    FD_SET(m_file, &readfs);
    FD_SET(m_file, &errorfs);

    if (WAIT_FOREVER == dwTimeoutInMS)
    {
        if (-1 == select(maxfd, &readfs, NULL, &errorfs, NULL))
        {
            RIL_LOG_CRITICAL("CFile::WaitForEvent() : ERROR : Select returned -1\r\n");
            return FALSE;
        }
    }
    else
    {
        struct timeval Time = msFromNowToTimeval(dwTimeoutInMS);

        if (-1 == select(maxfd, &readfs, NULL, &errorfs, &Time))
        {
            RIL_LOG_CRITICAL("CFile::WaitForEvent() : ERROR : Select returned -1\r\n");
            return FALSE;
        }
    }

    if (FD_ISSET(m_file, &readfs))
    {
        rdwFlags = FILE_EVENT_RXCHAR;
    }

    if (FD_ISSET(m_file, &errorfs))
    {
        RIL_LOG_CRITICAL("CFile::WaitForEvent() : ERROR : errorfs was set!\r\n");
        return FALSE;
    }

    return TRUE;
}

BOOL CFile::Open(CFile * pFile, const char * pszFileName, UINT32 dwAccessFlags, UINT32 dwOpenFlags, UINT32 dwOptFlags, BOOL fIsSocket)
{
    if (pFile)
    {
        return pFile->Open(pszFileName, dwAccessFlags, dwOpenFlags, dwOptFlags, fIsSocket);
    }
    else
    {
        RIL_LOG_CRITICAL("CFile::Open() : ERROR : pFile was NULL!\r\n");
        return FALSE;
    }
}

BOOL CFile::Close(CFile * pFile)
{
    if (pFile)
    {
        return pFile->Close();
    }
    else
    {
        RIL_LOG_CRITICAL("CFile::Close() : ERROR : pFile was NULL!\r\n");
        return FALSE;
    }
}

BOOL CFile::Read(CFile * pFile, void * pBuffer, UINT32 dwBytesToRead, UINT32 &rdwBytesRead)
{
    if (pFile)
    {
        return pFile->Read(pBuffer, dwBytesToRead, rdwBytesRead);
    }
    else
    {
        RIL_LOG_CRITICAL("CFile::Read() : ERROR : pFile was NULL!\r\n");
        return FALSE;
    }
}

BOOL CFile::Write(CFile * pFile, const void * pBuffer, UINT32 dwBytesToWrite, UINT32 &rdwBytesWritten)
{
    if (pFile)
    {
        return pFile->Write(pBuffer, dwBytesToWrite, rdwBytesWritten);
    }
    else
    {
        RIL_LOG_CRITICAL("CFile::Write() : ERROR : pFile was NULL!\r\n");
        return FALSE;
    }
}

BOOL CFile::WaitForEvent(CFile * pFile, UINT32 &rdwFlags, UINT32 dwTimeoutInMS)
{
    if (pFile)
    {
        return pFile->WaitForEvent(rdwFlags, dwTimeoutInMS);
    }
    else
    {
        RIL_LOG_CRITICAL("CFile::WaitForEvent() : ERROR : pFile was NULL!\r\n");
        return FALSE;
    }
}

int CFile::GetFD(CFile * pFile)
{
    if (pFile)
    {
        return pFile->m_file;
    }
    else
    {
        RIL_LOG_CRITICAL("CFile::GetFD() : ERROR : pFile was NULL!\r\n");
        return -1;
    }
}
