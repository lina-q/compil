#pragma once
#include <iostream>
#include <fstream>
#include <memory> //дл€ работы с динамической пам€тью
#include <vector>
#include <string>
#include <algorithm>
#include "LexicalAnalyzer.h"





struct ParseTreeNode 
{
    std::string value;
    std::vector<std::shared_ptr<ParseTreeNode>> children; //умный указатель, который управл€ет динамическими объектами в пам€ти

    ParseTreeNode(const std::string& val) : value(val) {}
};

//синтаксический анализатор
class SyntaxAnalyzer
{
public:
    SyntaxAnalyzer(LexicalAnalyzer& lexer, const std::string& outputFilename)
        : lexer(lexer), outputFilename(outputFilename), currentTokenIndex(0) 
    {
        tokens = lexer.getTokens();
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
            std::cerr << "—интаксическа€ ошибка: " 
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
    std::vector<std::string> call;

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
        auto nodeStart = std::make_shared<ParseTreeNode>("Begin");
        auto nodeEnd = std::make_shared<ParseTreeNode>("End");

        //проверка типа функции
        if (currentToken().type != TokenType::TYPE_INT && currentToken().type != TokenType::TYPE_FLOAT) {
            throw std::runtime_error("ќжидалс€ тип (int или float) ");
        }
        nodeStart->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // TYPE
        

        // проверка имени функции
        if (currentToken().type != TokenType::ID) {
            throw std::runtime_error("ќжидалось им€ функции ");
        }
        nodeStart->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // FunctionName

        
        if (currentToken().type != TokenType::LPAREN) {
            throw std::runtime_error("ќжидалась '(' ");
        }
        nextToken(); // '('

       
        if (currentToken().type != TokenType::RPAREN) {
            throw std::runtime_error("ќжидалась ')' ");
        }
        nextToken(); // ')'

       
        if (currentToken().type != TokenType::LBRACE) {
            throw std::runtime_error("ќжидалась '{' ");
        }
        nextToken(); // '{'

        // парсим описани€
        while (currentToken().type != TokenType::RBRACE)
        {
            if (currentToken().type == TokenType::TYPE_INT || currentToken().type == TokenType::TYPE_FLOAT) 
            {
                nodeStart->children.push_back(parseDescr());
            }
            else if (currentToken().type == TokenType::ID) 
            {
                nodeStart->children.push_back(parseDescr());
            }
            else if (currentToken().type == TokenType::RETURN) 
            {
                
                nodeEnd->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));

                nextToken(); 
                if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM
                    || currentToken().type == TokenType::ID)
                {
                    nodeEnd->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
                    nextToken(); 
                }

                if (currentToken().type != TokenType::SEMICOLON) 
                {
                    throw std::runtime_error("ќжидалась ';' после return ");
                }
                nextToken(); // ';'
            }
            else
            {
                throw std::runtime_error("ќжидалось объ€вление переменной, присваивание или return ");
            }
        }

        // ѕроверка на '}'
        if (currentToken().type != TokenType::RBRACE) 
        {
            throw std::runtime_error("ќжидалась '}' ");

        }
        nextToken(); // '}'
        node->children.push_back(nodeStart);
        node->children.push_back(nodeEnd);

       
        return node;
    }

    std::shared_ptr<ParseTreeNode> parseDescriptions() 
    {
        
        
        auto node = std::make_shared<ParseTreeNode>("Descriptions");

        while (currentToken().type != TokenType::RBRACE)// продолжать до конца блока
        { 
            
            node->children.push_back(parseDescr()); //парсим описание
        }

        return node;
    }

    //парсим Descr
    std::shared_ptr<ParseTreeNode> parseDescr()
    {

        auto node = std::make_shared<ParseTreeNode>("");

        if (currentToken().type == TokenType::TYPE_INT || currentToken().type == TokenType::TYPE_FLOAT) 
        {
           
            node = std::make_shared<ParseTreeNode>("Descriptions");
            
            nextToken();
            std::string dop = "\'";
            auto it = std::find(call.begin(), call.end(), currentToken().value);
            if (call.size() > 0 && it != call.end()) 
                throw std::runtime_error("“ака€ переменна€ " + dop + currentToken().value + dop + " уже существует ");
                
            call.push_back(currentToken().value);
            
            node->children.push_back(parseVarList());

            if (currentToken().type != TokenType::SEMICOLON)
            {
                
                throw std::runtime_error("ќжидалась ';' ");
            }
            nextToken(); // ';'
        }
        else if (currentToken().type == TokenType::ID) 
        {
      
            
            node = std::make_shared<ParseTreeNode>("Operators");
            auto nodeId = std::make_shared<ParseTreeNode>("Id");
            auto nodeOp = std::make_shared<ParseTreeNode>("Op");
            auto nodeC = std::make_shared<ParseTreeNode>("Const");
            
            
            nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nodeOp->children.push_back(nodeId);
           

            auto it = std::find(call.begin(), call.end(), currentToken().value);
             std::string dop = "\'";
            if(it == call.end())
                throw std::runtime_error("“ака€ переменна€ " + dop + currentToken().value + dop + " не была объ€влена");

            nextToken(); // ID
            
            if (currentToken().type == TokenType::ASSIGN )
            {
        
                nextToken(); // ѕереход к '='
                

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
                    throw std::runtime_error("ќжидалась значение или выражение после '=' ");
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
                        if(tk == 1) throw std::runtime_error("ќжидалась значение  "  );
                        else throw std::runtime_error("ќжидалась значение  "  );
                    }

            
                }

                
                node->children.push_back(nodeOp);

            }
            else 
            {
                throw std::runtime_error("ќжидалс€ оператор '=' ");
            }

           
            if (currentToken().type != TokenType::SEMICOLON) 
            {
                
                throw std::runtime_error("ќжидалась ';' ");
            }
            nextToken(); // ';'
        }
        else 
        {
            
            throw std::runtime_error("ќжидалось объ€вление переменной или присваивание ");
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

                throw std::runtime_error("ќжидалась '(' ");
            }
            nextToken();

            if (currentToken().type != TokenType::FLOAT_NUM)
            {
                throw std::runtime_error("ќжидалась вещественное число ");
            }
            nodeC->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));

            nextToken();

            if (currentToken().type != TokenType::RPAREN)
            {
                throw std::runtime_error("ќжидалась ')' ");
            }
            nextToken();
        }
        else if (currentToken().type == TokenType::FTOI)
        {
            
            node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            nextToken();


            if (currentToken().type != TokenType::LPAREN)
            {
                throw std::runtime_error("ќжидалась '(' ");
            }
            nextToken();

            if (currentToken().type != TokenType::INT_NUM)
            {
                throw std::runtime_error("ќжидалась целое  число ");
            }
            nodeC->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));

            nextToken();

            if (currentToken().type != TokenType::RPAREN)
            {
                throw std::runtime_error("ќжидалась ')' ");
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


    // ѕарсим VarList
    std::shared_ptr<ParseTreeNode> parseVarList() 
    {

        
        auto node = std::make_shared<ParseTreeNode>("VarList");
        auto nodeId = std::make_shared<ParseTreeNode>("Id");
       
        if (currentToken().type != TokenType::ID && previousToken.type != TokenType::RETURN) 
        {
            throw std::runtime_error("ќжидалось им€ переменной "  );
        }
        nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        nextToken(); // ID

        while (currentToken().type == TokenType::COMMA) 
        {
            nextToken(); // ','
            if (currentToken().type != TokenType::ID) 
            {
                
                throw std::runtime_error("ќжидалось им€ переменной "   );
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
            std::cerr << "ќшибка: Ќе удалось открыть файл дл€ записи." << std::endl;
            return;
        }
        outFile << std::string(depth * 2, ' ') << node->value << std::endl; // ќтступы
        for (const auto& child : node->children) 
        {
            outputParseTree(child, depth + 1); // –екурсивный вывод
        }
        outFile.close();
    }
};