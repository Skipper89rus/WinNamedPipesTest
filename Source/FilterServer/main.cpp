#include <string>
#include <vector>
#include <iostream>

#include "WinNamedPipe/WinNamedPipeProvider.h"
#include "WinNamedPipe/WinErrHelper.h"


const std::string PIPE_NAME = "filter_pipe";

int main()
{
   std::cout << "Filter server initializing..." << std::endl;
   WinPipeProvider pipe = WinPipeProvider();
   if ( !pipe.Create(PIPE_NAME) )
   {
      std::cerr << "Filter server initializing failed: can't create pipe" << std::endl;
      std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;
      return 1;
   }

   std::cout << "Waiting client connection..." << std::endl;
   pipe.WaitConnection();
   std::cout << "Client connected" << std::endl << std::endl;

   std::string command;
   do
   {
      std::string c;
      if ( !pipe.TryReadMsg(c) )
      {
         std::cout << "Failed to read message from client" << std::endl << std::endl;
         std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;

         std::cout << "Dropping client connection..." << std::endl;
         pipe.Disconnect();
         std::cout << "Waiting client connection..." << std::endl;
         pipe.WaitConnection();
         std::cout << "Client connected" << std::endl << std::endl;
         continue;
      }

      std::string out = c;
      if (c == "\r") // Enter
      {
         command.clear();
         std::cout << std::endl << out << std::endl;
      }
      else if (c == "\b")
      {
         if ( !command.empty() )
            command.pop_back();
         std::cout << "\b \b"; // Remove last symbol
      }
      else
      {
         command += c;
         std::cout << c;
      }

      if ( !pipe.SendMsg(out) )
      {
         std::cerr << "Sending test msg failed" << std::endl;
         std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;
      }
   } while (true);
   return 0;
}