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

    

    bool identifierExists(const std::string& identifier)
    {
        for (const auto& tokenPair : tokens)
        {
            const Token& token = tokenPair.first;


            if (token.type == TokenType::ID && token.value == identifier)
            {
                return true;
            }
        }
        return false;
    }

    //парсим функцию
    std::shared_ptr<ParseTreeNode> parseFunction()
    {

       

        auto node = std::make_shared<ParseTreeNode>("Function");
        auto nodeStart = std::make_shared<ParseTreeNode>("Begin");
        auto nodeEnd = std::make_shared<ParseTreeNode>("End");

        // Проверка типа функции
        if (currentToken().type != TokenType::TYPE_INT && currentToken().type != TokenType::TYPE_FLOAT) {
            throw std::runtime_error("Ожидался тип (int или float) ");
        }
        nodeStart->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // TYPE
        

        // Проверка имени функции
        if (currentToken().type != TokenType::ID) {
            throw std::runtime_error("Ожидалось имя функции ");
        }
        nodeStart->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // FunctionName

        // Проверка на '('
        if (currentToken().type != TokenType::LPAREN) {
            throw std::runtime_error("Ожидалась '(' ");
        }
        nextToken(); // '('

        // Проверка на ')'
        if (currentToken().type != TokenType::RPAREN) {
            throw std::runtime_error("Ожидалась ')' ");
        }
        nextToken(); // ')'

        // Проверка на '{'
        if (currentToken().type != TokenType::LBRACE) {
            throw std::runtime_error("Ожидалась '{' ");
        }
        nextToken(); // '{'

        // Парсим описания
        while (currentToken().type != TokenType::RBRACE) {
            if (currentToken().type == TokenType::TYPE_INT || currentToken().type == TokenType::TYPE_FLOAT) {
                nodeStart->children.push_back(parseDescr());
            }
            else if (currentToken().type == TokenType::ID) {
                // Обрабатываем присваивание
                nodeStart->children.push_back(parseDescr());
            }
            else if (currentToken().type == TokenType::RETURN) {
                
                nodeEnd->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
                
                
                nextToken(); 
                if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM || currentToken().type == TokenType::ID) {
                    nodeEnd->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
                    nextToken(); // Переход к значению
                }
                
                

                // Проверка на точку с запятой
                if (currentToken().type != TokenType::SEMICOLON) {
                    throw std::runtime_error("Ожидалась ';' после return ");
                }
                nextToken(); // ';'
            }
            else {
                throw std::runtime_error("Ожидалось объявление переменной, присваивание или return ");
            }
        }

        

        // Проверка на '}'
        if (currentToken().type != TokenType::RBRACE) 
        {
            throw std::runtime_error("Ожидалась '}' ");

        }
        nextToken(); // '}'
        node->children.push_back(nodeStart);
        node->children.push_back(nodeEnd);

       
        return node;
    }

    std::shared_ptr<ParseTreeNode> parseDescriptions() 
    {
        if (currentToken().type == TokenType::RETURN)
        {
            std::cout << "PUPUPU" << '\n';
            findReturn = true;
        }
        
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
        

        auto node = std::make_shared<ParseTreeNode>("");
        
       
        if (currentToken().type == TokenType::RETURN)  
        {
            std::cout << "PUPUPU" << '\n';
            findReturn = true;
        }

        // Проверка на тип переменной
        if (currentToken().type == TokenType::TYPE_INT || currentToken().type == TokenType::TYPE_FLOAT) {
            // Это объявление переменной
            node = std::make_shared<ParseTreeNode>("Descriptions");
            
            nextToken(); // TYPE

            // Парсим список переменных
            node->children.push_back(parseVarList());

            // Проверка на наличие точки с запятой
            if (currentToken().type != TokenType::SEMICOLON)
            {
                
                throw std::runtime_error("Ожидалась ';' ");
            }
            nextToken(); // ';'
        }
        else if (currentToken().type == TokenType::ID) {
            // Это присваивание
            
            node = std::make_shared<ParseTreeNode>("Operators");
            auto nodeId = std::make_shared<ParseTreeNode>("Id");
            auto nodeOp = std::make_shared<ParseTreeNode>("Op");
            auto nodeC = std::make_shared<ParseTreeNode>("Const");
            
            
            nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nodeOp->children.push_back(nodeId);
            nextToken(); // ID

            // Проверка на наличие оператора присваивания
            if (currentToken().type == TokenType::ASSIGN )
            {
        
                
                nextToken(); // Переход к '='
                //std::cout << currentToken().value << '\n';

                if (currentToken().type == TokenType::ITOF)
                {

                    nodeOp->children.push_back(parseSimpleExpr());

                }

                else if (currentToken().type == TokenType::FTOI)
                {

                    nodeOp->children.push_back(parseSimpleExpr());

                }
                else if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM)
                {
                    nodeOp->children.push_back(parseSimpleExpr());
           
                    
                }
                else
                {
            
                    throw std::runtime_error("Ожидалась значение или выражение после '=' ");
                }

                if (currentToken().type == TokenType::PLUS || currentToken().type == TokenType::MINUS)
                {
                    nodeOp->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
                    
                    int tk = 0;
                    if (currentToken().type == TokenType::PLUS) tk = 1;
                    else tk = 2;
                    nextToken();
            
                    if (currentToken().type == TokenType::ITOF)
                    {

                
                        nodeOp->children.push_back(parseSimpleExpr());

                    }

                    else if (currentToken().type == TokenType::FTOI)
                    {
                        nodeOp->children.push_back(parseSimpleExpr());

                    }
                    else if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM)
                    {
                        nodeOp->children.push_back(parseSimpleExpr());
                        
                    }
                    else
                    {
                        if(tk == 1) throw std::runtime_error("Ожидалась значение  "  );
                        else throw std::runtime_error("Ожидалась значение  "  );
                    }

            
                }

                
                
                node->children.push_back(nodeOp);
                
                

        
        
        
            }
            else 
            {
                throw std::runtime_error("Ожидался оператор '=' ");
            }

            // Проверка на наличие точки с запятой
            if (currentToken().type != TokenType::SEMICOLON) 
            {
                
                throw std::runtime_error("Ожидалась ';' ");
            }

            


            nextToken(); // ';'
        }
        else 
        {
            
            throw std::runtime_error("Ожидалось объявление переменной или присваивание ");
        }


        

        return node;
    }

    std::shared_ptr<ParseTreeNode> parseSimpleExpr()
    {
        auto node = std::make_shared<ParseTreeNode>("SimpleExpr");

        auto nodeC = std::make_shared<ParseTreeNode>("Const");



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
            nodeC->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));

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
            nodeC->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));

            nextToken();

            if (currentToken().type != TokenType::RPAREN)
            {
                throw std::runtime_error("Ожидалась ')' ");
            }
            nextToken();

        }
        else if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM)
        {
            nodeC->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            
            nextToken();
        }
        node->children.push_back(nodeC);
        
        
        return node;


    }


    // Парсим VarList
    std::shared_ptr<ParseTreeNode> parseVarList() 
    {

        
        auto node = std::make_shared<ParseTreeNode>("VarList");
        auto nodeId = std::make_shared<ParseTreeNode>("Id");
       
        if (currentToken().type != TokenType::ID && previousToken.type != TokenType::RETURN) 
        {
            //std::cout << currentToken().value << '\n';
            throw std::runtime_error("Ожидалось имя переменной "  );
        }
        nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // ID

        while (currentToken().type == TokenType::COMMA) 
        {
            nextToken(); // ','
            if (currentToken().type != TokenType::ID) 
            {
                
                throw std::runtime_error("Ожидалось имя переменной "   );
            }
            nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nextToken(); // ID
        }

        node->children.push_back(nodeId);

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