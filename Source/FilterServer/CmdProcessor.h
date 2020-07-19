#pragma once

#include <string>
#include <windows.h>

#include "WinNamedPipe/WinPipesIOHelper.h"

// Redirect subprocess IO
// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

class CmdProcessor
{
public:
   ~CmdProcessor()
   {
      deinit();
   }

   bool Init()
   {
      return createCmdIOPipes() && createCmdProc();
   }

   bool ExecCommand(const std::string& command, std::string& output)
   {
      if (_hCmdProc == nullptr || _hCmdThr == INVALID_HANDLE_VALUE)
         return false;
       return sendCommand(command) && getCommandOut(output);
   }

private:
   bool createCmdIOPipes()
   {
      SECURITY_ATTRIBUTES attrs = getSecurityAttrs();
      if ( !CreatePipe(&_hCmdOutR, &_hCmdOutW, &attrs, 0) ||
           !SetHandleInformation(_hCmdOutR, HANDLE_FLAG_INHERIT, 0) )
         return false;

      if ( !CreatePipe(&_hCmdInR, &_hCmdInW, &attrs, 0) ||
           !SetHandleInformation(_hCmdInW, HANDLE_FLAG_INHERIT, 0))
         return false;
      return true;
   }

   bool createCmdProc()
   {
      STARTUPINFO startInfo;
      ZeroMemory(&startInfo, sizeof(STARTUPINFO));
      startInfo.cb = sizeof(STARTUPINFO);
      startInfo.hStdError  = _hCmdOutW;
      startInfo.hStdOutput = _hCmdOutW;
      startInfo.hStdInput  = _hCmdInR;
      startInfo.dwFlags |= STARTF_USESTDHANDLES;

      PROCESS_INFORMATION procInfo;
      ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

      const DWORD cmdPathSz = 1024;
      CHAR cmdPath[cmdPathSz];
      if ( !GetEnvironmentVariable("COMSPEC", cmdPath, cmdPathSz) )
         return false;

      if ( !CreateProcess(cmdPath, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &startInfo, &procInfo))
         return false;

      _hCmdProc = procInfo.hProcess;
      _hCmdThr = procInfo.hThread;
      return true;
   }

   bool sendCommand(const std::string& command) const
   {
      std::string c = "\n\n" + command + "\n"; // TODO: How send one more command without "More?" question from CMD
      return win_pipes_io::write_to_pipe(_hCmdInW, c);
   }

   bool getCommandOut(std::string& out) const
   {
      std::string o;
      do
      {
         Sleep(250);

         DWORD availSz;
         if ( !PeekNamedPipe(_hCmdOutR, NULL, 0, NULL, &availSz, NULL) )
            return false;

         if (availSz == 0)
            break;

         if ( !win_pipes_io::read_from_pipe(_hCmdOutR, o) )
            return false;

         out += o;
      } while (true);
      return true;
   }

   SECURITY_ATTRIBUTES getSecurityAttrs() const
   {
      SECURITY_ATTRIBUTES attrs;
      attrs.nLength = sizeof(SECURITY_ATTRIBUTES);
      attrs.bInheritHandle = TRUE;
      attrs.lpSecurityDescriptor = NULL;
      return attrs;
   }

   void deinit()
   {
      TerminateProcess(_hCmdProc, NO_ERROR);

      closeHandle(_hCmdProc);
      closeHandle(_hCmdThr);

      closeHandle(_hCmdInR);
      closeHandle(_hCmdInW);
      closeHandle(_hCmdOutR);
      closeHandle(_hCmdOutW);
   }

   void closeHandle(HANDLE& h)
   {
      CloseHandle(h);
      h = nullptr;
   }

private:
   HANDLE _hCmdProc = nullptr;
   HANDLE _hCmdThr  = nullptr;

   HANDLE _hCmdInR  = nullptr;
   HANDLE _hCmdInW  = nullptr;
   HANDLE _hCmdOutR = nullptr;
   HANDLE _hCmdOutW = nullptr;
};