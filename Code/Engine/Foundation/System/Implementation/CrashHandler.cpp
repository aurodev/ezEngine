#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/MiniDumpUtils.h>
#include <Foundation/System/Process.h>
#include <Foundation/Time/Timestamp.h>

static void PrintHelper(const char* szString)
{
  ezLog::Printf("%s", szString);
}

//////////////////////////////////////////////////////////////////////////

ezCrashHandler* ezCrashHandler::s_pActiveHandler = nullptr;

ezCrashHandler::ezCrashHandler() = default;

ezCrashHandler::~ezCrashHandler()
{
  if (s_pActiveHandler == this)
  {
    SetCrashHandler(nullptr);
  }
}

ezCrashHandler* ezCrashHandler::GetCrashHandler()
{
  return s_pActiveHandler;
}

//////////////////////////////////////////////////////////////////////////

ezCrashHandler_WriteMiniDump ezCrashHandler_WriteMiniDump::g_Instance;

ezCrashHandler_WriteMiniDump::ezCrashHandler_WriteMiniDump() = default;

void ezCrashHandler_WriteMiniDump::SetFullDumpFilePath(ezStringView sFullAbsDumpFilePath)
{
  m_sDumpFilePath = sFullAbsDumpFilePath;
}

void ezCrashHandler_WriteMiniDump::SetDumpFilePath(ezStringView sAbsDirectoryPath, ezStringView sAppName, ezBitflags<PathFlags> flags)
{
  ezStringBuilder sOutputPath = sAbsDirectoryPath;

  if (flags.IsSet(PathFlags::AppendSubFolder))
  {
    sOutputPath.AppendPath("CrashDumps");
  }

  sOutputPath.AppendPath(sAppName);

  if (flags.IsSet(PathFlags::AppendDate))
  {
    const ezDateTime date = ezTimestamp::CurrentTimestamp();
    sOutputPath.AppendFormat("_{}", date);
  }

#if EZ_ENABLED(EZ_SUPPORTS_PROCESSES)
  if (flags.IsSet(PathFlags::AppendPID))
  {
    const ezUInt32 pid = ezProcess::GetCurrentProcessID();
    sOutputPath.AppendFormat("_{}", pid);
  }
#endif

  sOutputPath.Append(".dmp");

  SetFullDumpFilePath(sOutputPath);
}

void ezCrashHandler_WriteMiniDump::SetDumpFilePath(ezStringView sAppName, ezBitflags<PathFlags> flags)
{
  SetDumpFilePath(ezOSFile::GetApplicationDirectory(), sAppName, flags);
}

void ezCrashHandler_WriteMiniDump::HandleCrash(void* pOsSpecificData)
{
  bool crashDumpWritten = false;
  if (!m_sDumpFilePath.IsEmpty())
  {
#if EZ_ENABLED(EZ_SUPPORTS_CRASH_DUMPS)
    if (ezMiniDumpUtils::LaunchMiniDumpTool(m_sDumpFilePath).Failed())
    {
      ezLog::Print("Could not launch MiniDumpTool, trying to write crash-dump from crashed process directly.\n");

      crashDumpWritten = WriteOwnProcessMiniDump(pOsSpecificData);
    }
    else
    {
      crashDumpWritten = true;
    }
#else
    crashDumpWritten = WriteOwnProcessMiniDump(pOsSpecificData);
#endif
  }
  else
  {
    ezLog::Print("ezCrashHandler_WriteMiniDump: No dump-file location specified.\n");
  }

  PrintStackTrace(pOsSpecificData);

  if (crashDumpWritten)
  {
    ezLog::Printf("Application crashed. Crash-dump written to '%s'\n.", m_sDumpFilePath.GetData());
  }
}

//////////////////////////////////////////////////////////////////////////

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)
#  include <Foundation/System/Implementation/Win/CrashHandler_win.h>
#elif EZ_ENABLED(EZ_PLATFORM_OSX) || EZ_ENABLED(EZ_PLATFORM_LINUX) || EZ_ENABLED(EZ_PLATFORM_ANDROID)
#  include <Foundation/System/Implementation/Posix/CrashHandler_posix.h>
#else
#  error "ezCrashHandler is not implemented on current platform"
#endif


EZ_STATICLINK_FILE(Foundation, Foundation_System_Implementation_CrashHandler);
