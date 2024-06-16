#pragma once

#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <functional>

#ifdef __cpp_lib_format
# include <format>
#endif

#include "Result.hpp"

namespace ast {
    template<
        class TokenT,
        class CharT
    > struct basic_token {
        // regex must begin with ^
        std::basic_regex<CharT> regex;
        std::function<std::optional<int>(std::basic_string<CharT> arg, TokenT& token)> callback;
    };

    template<class TokenT>
    using token = basic_token<TokenT, char>;

    template<class TokenT>
    struct rule_option {
        std::vector<int> tokens;
        std::function<TokenT(std::vector<TokenT> tokens)> callback;
    };

    template<class TokenT>
    struct rule {
        int token;
        std::vector<rule_option<TokenT>> options;
    };

    template<
        class TokenT,
        class CharT
    > class basic_parser {
    private:
        std::optional<rule<TokenT>> find_rule(int token) {
            for (auto& token_rule : rules)
                if (token_rule.token == token)
                    return token_rule;

            return std::nullopt;
        }

        result::result<TokenT, std::string> resolve(std::basic_string<CharT>& string, int token) {
            std::vector<std::string> errors;

            std::optional<rule<TokenT>> optional_token_rule = find_rule(token);

            // base token
            if (!optional_token_rule.has_value()) {
                for (auto& cur_token : tokens) {
                    std::match_results<typename std::basic_string<CharT>::const_iterator> matches;
                    bool has_match = std::regex_search(string, matches, cur_token.regex);

                    if (has_match) {
                        std::basic_string<CharT> string_value = matches[0].str();
                        string = string.substr(string_value.size());

                        TokenT matched_token { };
                        std::optional<int> match_result = cur_token.callback(string_value, matched_token);

                        if (match_result.has_value()) {
                            if (match_result == token) {
                                return matched_token;
                            }
                            
#ifdef __cpp_lib_format
                            return std::format("syntax error: unexpected token \"{}\".", string_value);
#else
                            return "syntax error: unexpected token \"" + string_value + "\".";
#endif
                        }
                    }
                }

#ifdef __cpp_lib_format
                return std::format("syntax error: expected token {}.", token);
#else
                return "syntax error: expected token " + std::to_string(token) + ".";
#endif
            }

            rule<TokenT>& token_rule = optional_token_rule.value();

            // compound token
            for (auto& option : token_rule.options) {
                auto copy = string;
                std::vector<TokenT> tokens;

                bool error = false;

                for (auto& option_token : option.tokens) {
                    auto resolved = resolve(copy, option_token);

                    if (!resolved) {
                        errors.push_back(resolved.err);
                        error = true;
                        break;
                    }

                    tokens.push_back(resolved.ok);
                }

                if (!error) {
                    string = copy;
                    return option.callback(tokens);
                }
            }

#ifdef __cpp_lib_format
            std::string error_string = std::format("ast::basic_parser::resolve(std::basic_string<CharT>& string, int token)\n"
                                                   "syntax error: ran into {} errors while trying to resolve token {}.", errors.size(), token);
#else
            std::string error_string = "ast::basic_parser::resolve(std::basic_string<CharT>& string, int token)\n"
                                       "syntax error: ran into " + std::to_string(errors.size()) + " errors while trying to resolve token " + std::to_string(token) + ".\n";
#endif

            size_t i = 0;
            for (auto& error : errors) {
                error_string += "\n" + std::to_string(++i) + ": " + error;
            }
            
            error_string += "\n\ninput: \n" + string;

            return error_string;
        }

    protected:
        std::vector<basic_token<TokenT, CharT>> tokens;
        std::vector<rule<TokenT>> rules;
        int start;

    protected:
        basic_parser()                     = default;
        basic_parser(const basic_parser& ) = default;
        basic_parser(      basic_parser&&) = default;

        basic_parser(std::initializer_list<basic_token<TokenT, CharT>> tokens,
                     std::initializer_list<rule<TokenT>> rules,
                     int start) : tokens(tokens), rules(rules), start(start) { }

        TokenT do_parse(std::basic_string<CharT> string, bool silent = false) {
            auto result = resolve(string, start);

            if (result) {
                return result.ok;
            } else {
                if (!silent) {
                    std::cerr << "ast::basic_parser::do_parse(std::basic_string<CharT> string): an unrecoverable error occurred while parsing input:\n"
                              << result.err << std::endl;
                }

                throw std::runtime_error(result.err);
            }
        }
    };

    template<class TokenT>
    using parser = basic_parser<TokenT, char>;
}
