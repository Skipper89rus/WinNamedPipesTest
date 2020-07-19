#include <conio.h>
#include <string>
#include <iostream>

#include "WinNamedPipe/WinNamedPipeProvider.h"
#include "WinNamedPipe/WinConsoleHelper.h"

const std::string PIPE_NAME = "filter_pipe";

int main()
{
   std::cout << "Filter client initializing...." << std::endl;

   WinPipeProvider pipe = WinPipeProvider();
   if ( !win_console_helper::formatted_err_func_call([&pipe]() { return pipe.Connect(PIPE_NAME); },
                                                     "Filter client initializing failed: can't connect to pipe") )
      return 1;

   std::cout << "Type command..." << std::endl;
   while (const char c = _getch())
   {
      switch (c)
      {
      case '\r': std::cout << std::endl; break; // Enter
      case '\b': std::cout << "\b ";     break; // Remove last symbol
      }

      std::string out;
      if ( !win_console_helper::formatted_err_func_call([&pipe, c, &out]() { return pipe.Transact(PIPE_NAME, { c }, out); },
                                                        "Sending request to server failed") )
         continue;

      std::cout << out;
      if (c == '\r') // Next line on Enter
         std::cout << std::endl;
   }

   return 0;
}