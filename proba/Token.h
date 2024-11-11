#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>

//перечисление токенов
enum class TokenType 
{
    BEGIN, END, TYPE_INT, TYPE_FLOAT, RETURN,
    ITOF, FTOI, ID, INT_NUM, FLOAT_NUM,
    ASSIGN, PLUS, MINUS, COMMA, SEMICOLON,
    LPAREN, RPAREN, LBRACE, RBRACE, UNKNOWN
};

struct Token
{
    TokenType type;
    std::string value;
   

};
