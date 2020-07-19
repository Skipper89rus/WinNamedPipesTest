#pragma once

#include <string>
#include <windows.h>

#include "WinNamedPipe/WinPipesIOHelper.h"

// Redirect subprocess IO
// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

class CmdProcessor
{
public:
   bool ExecCommand(const std::string& command, std::string& output)
   {
      if ( !createCmdIOPipes() )
      {
         deinit();
         return false;
      }

      if ( !execCmdCommand(command) )
      {
         deinit();
         return false;
      }

      // Close handles to the child process and its primary thread
      // Some applications might keep these handles to monitor the status of the child process, for example
      closeHandle(_hCmdProc);
      closeHandle(_hCmdThr);
      // Close handles to the stdin and stdout pipes no longer needed by the child process
      // If they are not explicitly closed, there is no way to recognize that the child process has ended
      closeHandle(_hCmdOutW);
      closeHandle(_hCmdInR);
      // Close the pipe handle so the child process stops reading
      closeHandle(_hCmdInW);

      bool isSuccess = getCommandOut(output);
      deinit();
      return isSuccess;
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

   bool execCmdCommand(const std::string& command)
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

      LPSTR cmd = _strdup(("cmd /C " + command).c_str()); // CreateProcess uses LPSTR command line
      BOOL isSuccess = CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &startInfo, &procInfo);
      free(cmd);
      if ( !isSuccess )
         return false;

      _hCmdProc = procInfo.hProcess;
      _hCmdThr = procInfo.hThread;
      return true;
   }

   bool getCommandOut(std::string& out) const
   {
      std::string o;
      while (win_pipes_io::read_from_pipe(_hCmdOutR, o))
         out += o;
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