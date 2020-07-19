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

   std::cout << std::endl << "Type command..." << std::endl;
   while (const char c = _getch())
   {
      std::string response;
      if ( !win_console_helper::formatted_err_func_call([&pipe, c, &response]() { return pipe.Transact(PIPE_NAME, { c }, response); },
                                                        "Sending request to server failed") )
         continue;

      if (c == '\r') // If Enter was pressed, server response contains command output
      {
         std::cout << std::endl << std::endl;
         std::cout << "Command output:" << std::endl;
         std::cout << response          << std::endl << std::endl;
         std::cout << "Type command..." << std::endl;
      }
      else if (c == '\b')
         std::cout << "\b \b"; // Remove last symbol
      else // Otherwise, server response contains sent symbol
         std::cout << response;
   }

   return 0;
}