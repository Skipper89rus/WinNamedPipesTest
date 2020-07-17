#pragma once

#include <string>
#include <windows.h>

#include "WinPipesIOHelper.h"

// Using Pipes
// https://docs.microsoft.com/en-us/windows/win32/ipc/using-pipes

class WinPipeProvider
{
   static constexpr DWORD PIPE_WAIT_SEC = 10000;

public:
   ~WinPipeProvider()
   {
      closePipe();
   }

   bool Create(const std::string& pipeName)
   {
      _hPipe = CreateNamedPipe(buildFullPipeName(pipeName).c_str(), PIPE_ACCESS_DUPLEX,
                               PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                               PIPE_UNLIMITED_INSTANCES, win_pipes_io::MAX_MSG_LEN, win_pipes_io::MAX_MSG_LEN, 0, NULL);
      return _hPipe != INVALID_HANDLE_VALUE;
   }

   void Close()
   {
      closePipe();
   }

   bool Connect(const std::string& pipeName)
   {
      if ( checkPipe() )
         return false;

      const auto fullPipeName = buildFullPipeName(pipeName);
      _hPipe = CreateFile(fullPipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
      if ( _hPipe == INVALID_HANDLE_VALUE && GetLastError() != ERROR_PIPE_BUSY && !WaitNamedPipe(fullPipeName.c_str(), PIPE_WAIT_SEC) )
         return false;
      return changePipeMode(PIPE_READMODE_MESSAGE); // Switch to 'message-read' mode
   }

   bool Disconnect() const
   {
      return DisconnectNamedPipe(_hPipe) && changePipeMode(PIPE_WAIT); // Switch back to 'wait' mode
   }

   void WaitConnection() const
   {
      while ( !ConnectNamedPipe(_hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED )
      {
         Sleep(1000);
      }
   }

   bool Transact(const std::string& pipeName, const std::string& request, std::string& response) const
   {
      if ( !checkPipe() )
         return false;

      LPSTR lpszRequest = const_cast<char*>(request.c_str());

      CHAR responseBuf[win_pipes_io::MAX_MSG_LEN] = "";
      DWORD responseSz;
      BOOL isSuccess = TransactNamedPipe(_hPipe,
                                         lpszRequest, static_cast<DWORD>(request.size() + 1) * sizeof(CHAR),
                                         responseBuf, (win_pipes_io::MAX_MSG_LEN + 1) * sizeof(CHAR),
                                         &responseSz, NULL);
      if (!isSuccess && GetLastError() != ERROR_MORE_DATA)
         return false;

      response = responseBuf;
      return true;
   }

   bool SendMsg(const std::string& msg) const
   {
      return checkPipe() && win_pipes_io::write_to_pipe(_hPipe, msg);
   }

   bool TryReadMsg(std::string& msg) const
   {
      return checkPipe() && win_pipes_io::read_from_pipe(_hPipe, msg);
   }

private:
   static std::string buildFullPipeName(const std::string& pipeName)
   {
      return "\\\\.\\pipe\\" + pipeName;
   }

   bool checkPipe() const
   {
      return _hPipe != nullptr && _hPipe != INVALID_HANDLE_VALUE;
   }

   void closePipe()
   {
      if ( !checkPipe() )
         return;
      CloseHandle(_hPipe);
      _hPipe = nullptr;
   }

   bool changePipeMode(DWORD mode) const
   {
      return SetNamedPipeHandleState(_hPipe, &mode, NULL, NULL);
   }

private:
   HANDLE _hPipe = nullptr;
};