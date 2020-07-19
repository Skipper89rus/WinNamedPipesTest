#pragma once
#include <string>
#include <windows.h>

namespace win_console_helper
{
   inline std::string formatted_err(unsigned long errCode = GetLastError())
   {
      if (errCode == 0)
         return {};

      LPSTR msgBuf = nullptr;
      size_t msgSz = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL, errCode, NULL, (LPTSTR)&msgBuf, 0, NULL);

      std::string msg(msgBuf, msgSz);
      LocalFree(msgBuf);
      return msg;
   }

   template<typename TFunc>
   inline bool formatted_err_func_call(TFunc func, const std::string& errMsg)
   {
      if (func())
         return true;

      std::cerr << errMsg << std::endl;
      std::cerr << "Last error: " << formatted_err() << std::endl;
      return false;
   }
}