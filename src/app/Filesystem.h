#pragma once
#include <cctype>
#include <string>

namespace viscom
{
    bool is_not_digit(char c)
    {
        return !std::isdigit(c);
    }

    std::string parseFileName(const std::string& path)
    {
        size_t index = 0;
        auto length = 0;
        for (auto i = path.size() - 1; i > 0; --i)
        {
            if (path.at(i) == '/')
            {
                index = i + 1;
                break;
            }
            length++;
        }
        return path.substr(index, length);
    }

    bool comparePaths(const std::string& string1, const std::string& string2)
    {
        if (string1.empty()) return false;
        if (string2.empty()) return true;

        auto s1 = parseFileName(string1);
        auto s2 = parseFileName(string2);
        auto it1 = s1.begin(), it2 = s2.begin();
        auto n1 = -1;
        auto n2 = -1;
        std::string numberInString = "";
        for (auto& c : s1)
        {
            if (std::isdigit(c))
            {
                numberInString.push_back(c);
            }
        }
        if (!numberInString.empty())
            n1 = std::stoi(numberInString);
        numberInString.clear();
        for (auto& c : s2)
        {
            if (std::isdigit(c))
            {
                numberInString.push_back(c);
            }
        }
        if (!numberInString.empty())
            n2 = std::stoi(numberInString);

        if (n1 != n2) return n1 < n2;

        it1 = std::find_if(s1.begin(), s1.end(), is_not_digit);
        it2 = std::find_if(s2.begin(), s2.end(), is_not_digit);

        return std::lexicographical_compare(it1, s1.end(), it2, s2.end());
    }
}