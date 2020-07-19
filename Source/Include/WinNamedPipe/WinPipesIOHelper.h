#pragma once

namespace win_pipes_io
{
   const DWORD MAX_MSG_LEN = 1024;

   inline bool write_to_pipe(HANDLE h, const std::string& str)
   {
      DWORD writtenLen;
      BOOL isSuccess = WriteFile(h, str.c_str(), static_cast<DWORD>(str.size() + 1) * sizeof(CHAR), &writtenLen, NULL);
      return isSuccess && str.length() + 1 == writtenLen;
   }

   inline bool read_from_pipe(HANDLE h, std::string& str)
   {
      CHAR buf[MAX_MSG_LEN] = "";
      DWORD bufSz;
      BOOL isSuccess = ReadFile(h, buf, MAX_MSG_LEN, &bufSz, NULL);
      if (!isSuccess && GetLastError() != ERROR_MORE_DATA)
         return false;
      str = buf;
      return true;
   }
}