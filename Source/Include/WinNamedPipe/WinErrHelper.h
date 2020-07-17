#pragma once

namespace win_err_helper
{
   inline std::string fmt_err(unsigned long errCode = GetLastError())
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
}