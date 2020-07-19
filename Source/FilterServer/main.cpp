#include <string>
#include <vector>
#include <iostream>

#include "WinNamedPipe/WinNamedPipeProvider.h"
#include "WinNamedPipe/WinConsoleHelper.h"

#include "CmdFilter.h"
#include "CmdProcessor.h"


const std::string PIPE_NAME = "filter_pipe";

const std::vector<std::string> CMD_FILTER_REGEXES =
{
   "ping\\s+yandex.*", "dir"
};


inline bool try_exec_command(CmdProcessor* pCmdProcessor, const std::string& command, std::string& out)
{
   return win_console_helper::formatted_err_func_call(
      [pCmdProcessor, &command, &out]() { return pCmdProcessor->ExecCommand(command, out); },
      "Commnd '" + command + "' execution failed" );
}

int main()
{
   std::cout << "Filter server initializing..." << std::endl;

   WinPipeProvider pipe = WinPipeProvider();
   // Init server-side pipe
   if ( !win_console_helper::formatted_err_func_call([&pipe]() { return pipe.Create(PIPE_NAME); },
                                                    "Filter client initializing failed: can't connect to pipe") )
      return 1;

   CmdFilter cmdFilter = CmdFilter();
   // Init commands filter
   if ( !win_console_helper::formatted_err_func_call([&cmdFilter]() { return cmdFilter.Init(CMD_FILTER_REGEXES); },
                                                     "Filter server initializing failed: some of filter expressions are invalid") )
      return 1;

   CmdProcessor cmdProcessor = CmdProcessor();
   // Init commands processor
   if ( !win_console_helper::formatted_err_func_call([&cmdProcessor]() { return cmdProcessor.Init(); },
                                                     "Filter server initializing failed: can't init commands processor") )
      return 1;

   std::cout << "Waiting client connection..." << std::endl;
   pipe.WaitConnection();
   std::cout << "Client connected" << std::endl << std::endl;

   std::string command;
   do
   {
      std::string c;
      // Reading msg from client
      if ( !win_console_helper::formatted_err_func_call([&pipe, &c]() { return pipe.TryReadMsg(c); },
                                                        "Failed to read message from client") )
      {
         std::cout << "Dropping client connection..." << std::endl;
         if ( !win_console_helper::formatted_err_func_call([&pipe]() { return pipe.Disconnect(); },
                                                           "Dropping client connection failed") )

         // Wait client connaction again
         std::cout << "Waiting client connection..." << std::endl;
         pipe.WaitConnection();
         std::cout << "Client connected" << std::endl << std::endl;

         command.clear();
         continue;
      }

      std::string out = c;
      if (c == "\r") // Enter
      {
         if (cmdFilter.IsForbidden(command))
            out = "Commnd '" + command + "' forbidden";
         else if ( !try_exec_command(&cmdProcessor, command, out) )
            out = "Commnd '" + command + "' execution failed";
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

      win_console_helper::formatted_err_func_call([&pipe, &out]() { return pipe.SendMsg(out); },
                                                  "Sending response to client failed");

   } while (true);
   return 0;
}