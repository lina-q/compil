#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include "HashTable.h"
#include "Token.h"

// Лексический анализатор
class LexicalAnalyzer
{
public:
    // Конструктор с передачей имени файла и хэш-таблицы
    LexicalAnalyzer(const std::string& filename, HashTable& hashTable)
        : position(0), hashTable(hashTable) // Инициализация позиции и ссылки на хэш-таблицу
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Файл не открыт " << filename << std::endl;
            return;
        }
        input.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }

    // Токенизация
    void tokenize()
    {
        Token lastToken = { TokenType::UNKNOWN, "" }; // хранит последний токен
       

        while (position < input.size())
        {
            char currentChar = input[position];

            if (currentChar == '\n') ++line;
            
            // Если это пробел
            if (isspace(currentChar))
            {
                ++position;
                continue;
            }

            // Проверяем на буквы
            if (isalpha(currentChar))
            {
                Token currentToken = parseIdentifierOrKeyword(); 
                hashTable.insert(currentToken); 
                tokens.push_back(std::make_pair(currentToken, line));
                lastToken = currentToken;
            }
            else if (isdigit(currentChar))
            {
                Token currentToken = parseNumber();
                hashTable.insert(currentToken);
                tokens.push_back(std::make_pair(currentToken, line));
                lastToken = currentToken;
            }
            else
            {
                Token currentToken = parseOperator();
                hashTable.insert(currentToken);
                tokens.push_back(std::make_pair(currentToken, line));
                lastToken = currentToken;
            }
        }
    }
    std::vector<std::pair<Token, int>> getTokens() {
        return tokens; // Возвращаем вектор токенов
    }

private:
    std::string input; 
    size_t position; 
    HashTable& hashTable; 
    std::vector<std::pair<Token, int>> tokens; // Вектор для хранения токенов
    std::vector<int> lines;
    int line = 1;
    

    Token parseIdentifierOrKeyword()
    {
        size_t start = position;
        bool hasDigit = false; // наличие цифры

        while (position < input.size() &&
           (isalnum(input[position]) || input[position] == '_')) // только латинские буквы и цифры
    {
        if (isdigit(input[position])) {
            hasDigit = true;
        }
        ++position;
    }

        std::string value = input.substr(start, position - start);

        // Если идентификатор содержит цифры, выводим ошибку
        if (hasDigit) {
            std::cerr << "Error: Идентификатор '" << value << "' не может содержать цифры." << std::endl;
            return { TokenType::UNKNOWN, value }; // Возвращаем UNKNOWN токен
        }

        TokenType type = identifyKeyword(value);
        return { type, value };
    }
    //проверка, является ли состояние ключевым
    TokenType identifyKeyword(const std::string& value)
    {
        static std::unordered_map<std::string, TokenType> keywords =
        {
            
            {"int", TokenType::TYPE_INT}, {"float", TokenType::TYPE_FLOAT},
            {"return", TokenType::RETURN}, {"itof", TokenType::ITOF},
            {"ftoi", TokenType::FTOI}
        };
        auto it = keywords.find(value);
        if (it != keywords.end())
        {
            return it->second;
        }
        return TokenType::ID;
    }

    
    // Проверка, является ли это числом
    Token parseNumber()
    {
        size_t start = position; 
        bool hasDecimalPoint = false; //наличия десятичной точки

        while (position < input.size() && isdigit(input[position]))
        {
            position++;
        }

        // Проверяем на наличие десятичной точки
        if (position < input.size() && input[position] == '.')
        {
            hasDecimalPoint = true; 
            position++; 

            while (position < input.size() && isdigit(input[position]))
            {
                position++;
            }

            //проверяем, есть ли еще одна точка после этой
            if (position < input.size() && input[position] == '.')
            {
                position++;
                std::cerr << "Error: Некорректный формат числа '" << input.substr(start, position - start + 1) << "'." << std::endl;
                return { TokenType::UNKNOWN, input.substr(start, position - start +1) }; // возвращаем UNKNOWN токен
            }

            return { TokenType::FLOAT_NUM, input.substr(start, position - start) };
        }

        // Если не было десятичной точки, возвращаем целое число
        return { TokenType::INT_NUM, input.substr(start, position - start) }; 
    }

    // Проверка операторов
    Token parseOperator()
    {
        char currentChar = input[position++];
        switch (currentChar)
        {
        case '=': return { TokenType::ASSIGN, "=" };
        case '+': return { TokenType::PLUS, "+" };
        case '-': return { TokenType::MINUS, "-" };
        case ',': return { TokenType::COMMA, "," };
        case ';': return { TokenType::SEMICOLON, ";" };
        case '(': return { TokenType::LPAREN, "(" };
        case ')': return { TokenType::RPAREN, ")" };
        case '{': return { TokenType::LBRACE, "{" };
        case '}': return { TokenType::RBRACE, "}" };
        default: return { TokenType::UNKNOWN, std::string(1, currentChar) };
        }
    }
};