#pragma once

#include <string>
#include <vector>
#include <regex>
#include <algorithm>

class CmdFilter
{
public:
   bool Init(const std::vector<std::string>& forbiddenRegexes)
   {
      std::vector<std::regex> forbidden;
      forbidden.reserve(forbiddenRegexes.size());
      try
      {
         for (const auto& rxStr : forbiddenRegexes)
            forbidden.emplace_back(rxStr);
      }
      catch (...)
      {
         // TODO: Extend logging
         return false;
      }
      _forbidden = std::move(forbidden);
      return true;
   }

   bool IsForbidden(const std::string& command) const
   {
      return std::any_of(_forbidden.begin(), _forbidden.end(), [&command](const std::regex& rx) { return std::regex_match(command, rx); });
   }

private:
   std::vector<std::regex> _forbidden;
};