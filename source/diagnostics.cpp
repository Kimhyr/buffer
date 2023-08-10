#include "diagnostics.h"

void _record(std::string_view file,
                    std::size_t      line,
                    record_type      type,
                    std::string_view message)
{
    std::chrono::time_point time = std::chrono::system_clock::now();
    std::string_view type_string;
    switch (type) {
    case record_type::note:
        type_string = "note";
        break;
    case record_type::warning:
        type_string = "warn";
        break;
    case record_type::failure:
        type_string = "fail";
        break;
    }
    std::string format = std::format(
        "{:%X} [{}] {}:{}: {}",
        time, type_string, file, line, message);
    std::cerr << format << std::endl;
}

