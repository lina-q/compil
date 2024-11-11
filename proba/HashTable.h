#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "Token.h"
#include <iostream>
#include <fstream>
#include <string>

const int TABLE_SIZE = 100; // размер таблицы
const int MAX_ENTRIES = 100000; // максимальное количество записей 

// информация об одном токене
struct OneToken
{
    TokenType type;
    std::string lexeme;
    OneToken* next; // Указатель на следующий элемент в цепочке
};

class HashTable
{
private:
    OneToken* table[TABLE_SIZE];
    OneToken* insertionOrder[MAX_ENTRIES]; // массив для хранения порядка вставки
    int insertionCount; // количество вставленных элементов

public:
    // Конструктор
    HashTable() : insertionCount(0)
    {
        for (int i = 0; i < TABLE_SIZE; ++i)
        {
            table[i] = nullptr;
        }
        for (int i = 0; i < MAX_ENTRIES; ++i)
        {
            insertionOrder[i] = nullptr;
        }
    }

    // Деструктор
    ~HashTable()
    {
        for (int i = 0; i < TABLE_SIZE; ++i)
        {
            OneToken* current = table[i];
            while (current != nullptr) {
                OneToken* temp = current;
                current = current->next;
                delete temp;
            }
        }
    }

    
    int hash(const std::string& lexeme) const
    {
        unsigned long hashValue = 5381; 
        for (char c : lexeme) {
            hashValue = ((hashValue << 5) + hashValue) + c; //сдвиг на 5 битов это то же что и hashValue * 33 + c
        }
        return hashValue % TABLE_SIZE; 
    }

    
    void insert(const Token& token)
    {
        std::string lexeme = token.value;
        TokenType type = token.type;
        int index = hash(lexeme);

        OneToken* current = table[index];

        // Проверка на наличие токена в таблице
        while (current != nullptr)
        {
            if (current->type == type && current->lexeme == lexeme) {
                return; // Если элемент уже существует, ничего не делаем
            }
            current = current->next;
        }

        // Если элемент не найден, добавляем его
        OneToken* newEntry = new OneToken{ type, lexeme, table[index] };
        table[index] = newEntry; // Вставляем в начало цепочки
        if (insertionCount < MAX_ENTRIES)
        {
            insertionOrder[insertionCount++] = newEntry;
        }
    }

    // Вывод в файл
    void print() const
    {
        std::ofstream fout("output.txt");

        if (fout.is_open())
        {
            fout << "BEGIN" << '\n';
            fout << '\n';
            for (int i = 0; i < insertionCount; ++i)
            {
                const OneToken* oneToken = insertionOrder[i];
                if (oneToken)
                {
                    fout << tokenTypeToString(oneToken->type) << " | " << oneToken->lexeme << "  |  " << hash(oneToken->lexeme) << '\n';
                }
            }
            fout << '\n';
            fout << "END" << '\n';

            fout << '\n';
            fout << '\n';
            fout.close();
        }
        else
        {
            std::cerr << "Ошибка открытия файла output.txt" << '\n';
        }
    }

    // Перевод токена в строку
    std::string tokenTypeToString(TokenType type) const {
        switch (type) {
        case TokenType::BEGIN: return "BEGIN";
        case TokenType::END: return "END";
        case TokenType::TYPE_INT: return "TYPE_INT";
        case TokenType::TYPE_FLOAT: return "TYPE_FLOAT";
        case TokenType::RETURN: return "RETURN";
        case TokenType::ITOF: return "ITOF";
        case TokenType::FTOI: return "FTOI";
        case TokenType::ID: return "ID";
        case TokenType::INT_NUM: return "INT_NUM";
        case TokenType::FLOAT_NUM: return "FLOAT_NUM";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        default: return "UNKNOWN";
        }
    }
};

#endif // HASH_TABLE_H