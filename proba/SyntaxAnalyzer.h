#pragma once
#include <iostream>
#include <fstream>
#include <memory> //для работы с динамической памятью
#include <vector>
#include <string>
#include "LexicalAnalyzer.h"





struct ParseTreeNode 
{
    std::string value;
    std::vector<std::shared_ptr<ParseTreeNode>> children; //умный указатель, который управляет динамическими объектами в памяти

    ParseTreeNode(const std::string& val) : value(val) {}
};

// Синтаксический анализатор
class SyntaxAnalyzer
{
public:
    SyntaxAnalyzer(LexicalAnalyzer& lexer, const std::string& outputFilename)
        : lexer(lexer), outputFilename(outputFilename), currentTokenIndex(0) 
    {
        tokens = lexer.getTokens(); //получаем токены из лексического анализатора
        previousToken = { TokenType::UNKNOWN, "" }; 
    }

    void parse() 
    {
        try 
        {
            root = parseFunction();
            
            outputParseTree(root, 0); 
        }
        catch (const std::runtime_error& e) 
        {
            std::string sim = "\'";
            std::cerr << "Синтаксическая ошибка: " 
                << "| строка " + std::to_string(currentLine()) + " | " << e.what()
                << " после " + sim + previousToken.value << sim << '\n';
        }
    }

private:
    LexicalAnalyzer& lexer;
    std::string outputFilename;
    std::vector<std::pair<Token, int>> tokens; 
    size_t currentTokenIndex;  
    std::shared_ptr<ParseTreeNode> root; 
    Token previousToken; 
    bool findReturn = false;

    bool lBrace = false;
    bool rBrace = false;

   
    Token currentToken() 
    {
        if (currentTokenIndex < tokens.size()) 
        {
            return tokens[currentTokenIndex].first;
        }
        return { TokenType::UNKNOWN, "" }; //  UNKNOWN  если нет токенов
    }

    int currentLine()
    {
        return tokens[currentTokenIndex].second;
    }


    
    void nextToken()
    {
        if (currentTokenIndex < tokens.size())
        {
            previousToken = currentToken(); 
            ++currentTokenIndex;
        }
    }



    //парсим функцию
    std::shared_ptr<ParseTreeNode> parseFunction()
    {
        auto node = std::make_shared<ParseTreeNode>("Function");

        

   
        
        if (currentToken().type != TokenType::TYPE_INT && currentToken().type != TokenType::TYPE_FLOAT)
        {
            throw std::runtime_error("Ожидался тип (int или float) "  );
        }
        node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // TYPE


       
        if (currentToken().type != TokenType::ID ) 
        {
            throw std::runtime_error("Ожидалось имя функции "  );
        }
        node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // FunctionName


        
        if (currentToken().type != TokenType::LPAREN && previousToken.type == TokenType::ID)
        {
            throw std::runtime_error("Ожидалась '(' "  );
        }
        nextToken(); // '('



        if (currentToken().type != TokenType::RPAREN)
        {
            throw std::runtime_error("Ожидалась ')' "  );
        }
        nextToken(); // ')'
  
        if (currentToken().type != TokenType::LBRACE && previousToken.type == TokenType::RPAREN) 
        {
            throw std::runtime_error("Ожидалась '{' "  );
        }
        nextToken(); // '{'


        //парсим Descriptions
        while (currentToken().type != TokenType::RBRACE) 
        {
            node->children.push_back(parseDescriptions());
        }

        if (currentToken().type != TokenType::RBRACE )
        {
            throw std::runtime_error("Ожидалась '}' "  );
        }
        nextToken(); // '}'

        return node;
    }

    std::shared_ptr<ParseTreeNode> parseDescriptions() 
    {
        auto node = std::make_shared<ParseTreeNode>("Descriptions");

        while (currentToken().type != TokenType::RBRACE)
        { // продолжать до конца блока
            node->children.push_back(parseDescr()); //парсим описание
        }

        return node;
    }

    // Парсим Descr
    std::shared_ptr<ParseTreeNode> parseDescr()
    {

        

        if (currentToken().type == TokenType::RETURN)
        {
            findReturn = true;
           
        }
        auto node = std::make_shared<ParseTreeNode>("Descr");
        
       
        if (currentToken().type != TokenType::TYPE_INT && currentToken().type != TokenType::TYPE_FLOAT &&
            currentToken().type != TokenType::RETURN)
        {

            
            
            if(findReturn) throw std::runtime_error("Ожидался '}' " );
            
             else throw std::runtime_error("Ожидался тип (int или float) " );
        }
        node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // TYPE

        // Парсим список переменных
        node->children.push_back(parseVarList());

        // Проверяем на наличие оператора присваивания
        if (currentToken().type == TokenType::ASSIGN) 
        {
            

            nextToken(); // Переход к '='


            if (currentToken().type == TokenType::ITOF)
            {

                node->children.push_back(parseSimpleExpr());

            }

            else if (currentToken().type == TokenType::FTOI)
            {
  
                node->children.push_back(parseSimpleExpr());

            }
            else if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM)
            {
               
                nextToken();
            }
            else
            {
                
                throw std::runtime_error("Ожидалась значение или выражение после '=' ");
            }

            if (currentToken().type == TokenType::PLUS || currentToken().type == TokenType::MINUS)
            {
                int tk = 0;
                if (currentToken().type == TokenType::PLUS) tk = 1;
                else tk = 2;
                nextToken();
                
                if (currentToken().type == TokenType::ITOF)
                {

                    
                    node->children.push_back(parseSimpleExpr());

                }

                else if (currentToken().type == TokenType::FTOI)
                {
                    node->children.push_back(parseSimpleExpr());

                }
                else if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM)
                {

                    nextToken();
                }
                else
                {
                    if(tk == 1) throw std::runtime_error("Ожидалась значение или выражение после '+' "  );
                    else throw std::runtime_error("Ожидалась значение или выражение после '-'  "  );
                }

                
            }
            
            
            
        }
       

        // Проверка на наличие точки с запятой
        if (currentToken().type != TokenType::SEMICOLON) 
        {
            throw std::runtime_error("Ожидалась ';'   "  );
        }
        nextToken(); // ';'

        return node;
    }

    std::shared_ptr<ParseTreeNode> parseSimpleExpr() {
        auto node = std::make_shared<ParseTreeNode>("SimpleExpr");



        if (currentToken().type == TokenType::ITOF)

        {
            node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nextToken();


            if (currentToken().type != TokenType::LPAREN)
            {

                throw std::runtime_error("Ожидалась '(' ");
            }
            nextToken();

            if (currentToken().type != TokenType::FLOAT_NUM)
            {
                throw std::runtime_error("Ожидалась вещественное число ");
            }


            nextToken();

            if (currentToken().type != TokenType::RPAREN)
            {
                throw std::runtime_error("Ожидалась ')' ");
            }
            nextToken();
        }
        else if (currentToken().type == TokenType::FTOI)
        {
            
            node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nextToken();


            if (currentToken().type != TokenType::LPAREN)
            {
                throw std::runtime_error("Ожидалась '(' ");
            }
            nextToken();

            if (currentToken().type != TokenType::INT_NUM)
            {
                throw std::runtime_error("Ожидалась целое  число ");
            }


            nextToken();

            if (currentToken().type != TokenType::RPAREN)
            {
                throw std::runtime_error("Ожидалась ')' ");
            }
            nextToken();

        }
        

        return node;


    }


    // Парсим VarList
    std::shared_ptr<ParseTreeNode> parseVarList() 
    {
        auto node = std::make_shared<ParseTreeNode>("VarList");
        if (currentToken().type != TokenType::ID && previousToken.type != TokenType::RETURN) 
        {
            throw std::runtime_error("Ожидалось имя переменной "  );
        }
        node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // ID

        while (currentToken().type == TokenType::COMMA) 
        {
            nextToken(); // ','
            if (currentToken().type != TokenType::ID) 
            {
                throw std::runtime_error("Ожидалось имя переменной "   );
            }
            node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nextToken(); // ID
        }

        return node;

       
    }

    //вывод дерева разбора в файл
    void outputParseTree(const std::shared_ptr<ParseTreeNode>& node, int depth) 
    {
        std::ofstream outFile(outputFilename, std::ios::app);
        if (!outFile.is_open())
        {
            std::cerr << "Ошибка: Не удалось открыть файл для записи." << std::endl;
            return;
        }
        outFile << std::string(depth * 2, ' ') << node->value << std::endl; // Отступы
        for (const auto& child : node->children) 
        {
            outputParseTree(child, depth + 1); // Рекурсивный вывод
        }
        outFile.close();
    }
};