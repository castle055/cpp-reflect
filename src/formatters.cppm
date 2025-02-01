// Copyright (c) 2024, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  formatters.cppm
 *! \brief
 *!
 */

export module reflect:formatters;

export import std;

import :types;

// template<refl::Reflected R, typename CharT>
// struct std::formatter<R, CharT> {
// // private:
//   static constexpr bool parse_field_fmt(
//     format_parse_context::iterator &first,
//     format_parse_context::iterator last,
//     const std::string_view field_name
//   ) {
//     const auto start = first;

//     std::size_t i = 0;
//     while (first != last && i < field_name.size()) {
//       if (*first != field_name[i]) {
//         first = start;
//         return false;
//       }
//       ++first;
//       ++i;
//     }

//     return true;
//   }

// // public:
//   constexpr auto parse(
//     format_parse_context &parse_ctx
//   ) {
//     if (parse_ctx.begin() == parse_ctx.end() || *parse_ctx.begin() == '}') {
//       return parse_ctx.begin();
//     }
//     // parse_ctx.next_arg_id();
//     auto last  = parse_ctx.end();
//     auto first = parse_ctx.begin();
//     // std::cout << "[BEGIN PARSE]" << std::endl;
//     // for (auto &it: parse_ctx) {
//     //   std::cout << it << std::endl;
//     // }
//     // std::cout << "[END PARSE]" << std::endl;
//     if (parse_field_fmt(first, last, "timestamp")) {
//       entry_field = LOG::entry_field_e::TIMESTAMP;
//       return (first);
//     } else if (parse_field_fmt(first, last, "level")) {
//       entry_field = LOG::entry_field_e::LEVEL;
//       return (first);
//     } else if (parse_field_fmt(first, last, "path")) {
//       entry_field = LOG::entry_field_e::PATH;
//       return (first);
//     } else if (parse_field_fmt(first, last, "line")) {
//       entry_field = LOG::entry_field_e::LINENUM;
//       return (first);
//     } else if (parse_field_fmt(first, last, "function")) {
//       entry_field = LOG::entry_field_e::FUNCTION;
//       return (first);
//     } else if (parse_field_fmt(first, last, "message")) {
//       entry_field = LOG::entry_field_e::MESSAGE;
//       return (first);
//     } else {
//       // throw std::format_error {"format error: invalid format-spec for log entry"};
//       return (last);
//     }
//   }

//   template<typename FC>
//   typename FC::iterator format(
//     const LOG::entry_t &entry,
//     FC &fmt_ctx
//   ) const {
//     std::ostringstream ss { };
//     switch (entry_field) {
//       case LOG::entry_field_e::NONE:
//         break;
//       case LOG::entry_field_e::TIMESTAMP:
//         ss << std::format("{0:%Y-%m-%d} {0:%H:%M:%S}", entry.timestamp);
//         break;
//       case LOG::entry_field_e::LEVEL:
//         ss << std::format("{}", entry.level.name);
//         break;
//       case LOG::entry_field_e::PATH:
//         ss << std::format("{}", entry.path.string());
//         break;
//       case LOG::entry_field_e::LINENUM:
//         ss << std::format("{}", entry.linenum);
//         break;
//       case LOG::entry_field_e::FUNCTION:
//         ss << std::format("{}", entry.function);
//         break;
//       case LOG::entry_field_e::MESSAGE:
//         ss << entry.message;
//         break;
//     }
//     for (const char &c: ss.view()) {
//       *fmt_ctx.out()++ = c;
//     }
//     return fmt_ctx.out();
//   }

// // private:
//   LOG::entry_field_e entry_field = LOG::entry_field_e::NONE;
// };
