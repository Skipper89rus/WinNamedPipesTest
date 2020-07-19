#include <string>
#include <vector>
#include <iostream>

#include "WinNamedPipe/WinNamedPipeProvider.h"
#include "WinNamedPipe/WinErrHelper.h"

#include "CmdFilter.h"
#include "CmdProcessor.h"


const std::string PIPE_NAME = "filter_pipe";

const std::vector<std::string> CMD_FILTER_REGEXES =
{
   "ping\\s+yandex.*", "dir"
};


inline bool try_exec_command(CmdProcessor* pCmdProcessor, const std::string& command, std::string& out)
{
   if (pCmdProcessor->ExecCommand(command, out))
      return true;
   std::cerr << "Commnd '" + command + "' execution failed" << std::endl;
   std::cout << win_err_helper::fmt_err() << std::endl;
   return false;
}

int main()
{
   std::cout << "Filter server initializing..." << std::endl;
   WinPipeProvider pipe = WinPipeProvider();

   CmdFilter cmdFilter = CmdFilter();
   if ( !cmdFilter.Init(CMD_FILTER_REGEXES) )
   {
      std::cerr << "Filter server initializing failed: some of filter expressions are invalid" << std::endl;
      std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;
      return 1;
   }

   CmdProcessor cmdProcessor = CmdProcessor();
   cmdProcessor.Init();
   cmdProcessor.~CmdProcessor();

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

         command.clear();
         continue;
      }

      std::string out = c;
      if (c == "\r") // Enter
      {
         if (cmdFilter.IsForbidden(command))
            out = "Commnd '" + command + "' forbidden";
         else if (!try_exec_command(&cmdProcessor, command, out))
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

      if ( !pipe.SendMsg(out) )
      {
         std::cerr << "Sending test msg failed" << std::endl;
         std::cerr << "Error: " << win_err_helper::fmt_err() << std::endl;
      }
   } while (true);
   return 0;
}