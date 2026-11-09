#ifndef DS_TEXT_H
#define DS_TEXT_H

#include <string>
#include <vector>

namespace ds
{

// Текст для пустой структуры
inline const char* emptyText()
{
    return "(empty)";
}

// Склеивает строки через разделитель
inline std::string join(const std::vector<std::string>& items, const std::string& separator)
{
    std::string result;
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        if (i > 0)
        {
            result += separator;
        }
        result += items[i];
    }
    return result;
}

} // namespace ds

#endif
