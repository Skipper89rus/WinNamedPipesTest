#include <conio.h>
#include <string>
#include <iostream>

#include "WinNamedPipe/WinNamedPipeProvider.h"
#include "WinNamedPipe/WinErrHelper.h"

const std::string PIPE_NAME = "filter_pipe";

int main()
{
   std::cout << "Filter client initializing...." << std::endl;
   WinPipeProvider pipe = WinPipeProvider();

   if ( !pipe.Connect(PIPE_NAME) )
   {
      std::cerr << "Filter client initializing failed: can't connect to pipe" << std::endl;
      std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;
      return 1;
   }

   std::cout << "Type command..." << std::endl;
   while (const char c = _getch())
   {
      switch (c)
      {
      case '\r': std::cout << std::endl; break; // Enter
      case '\b': std::cout << "\b ";     break; // Remove last symbol
      }

      std::string out;
      if ( !pipe.Transact(PIPE_NAME, { c }, out) )
      {
         std::cerr << "Sending test msg failed" << std::endl;
         std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;
         continue;
      }

      std::cout << out;
      if (c == '\r') // Next line on Enter
         std::cout << std::endl;
   }

   return 0;
}