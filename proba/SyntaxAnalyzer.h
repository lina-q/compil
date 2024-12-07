#pragma once
#include <iostream>
#include <fstream>
#include <memory> //для работы с динамической памятью
#include <vector>
#include <string>
#include <algorithm>
#include "LexicalAnalyzer.h"
#include <utility>
#include <unordered_map>



struct ParseTreeNode 
{
    std::string value;
    std::vector<std::shared_ptr<ParseTreeNode>> children; //умный указатель, который управляет динамическими объектами в памяти

    ParseTreeNode(const std::string& val) : value(val) {}
};

//синтаксический анализатор
class SyntaxAnalyzer
{
public:
    std::shared_ptr<ParseTreeNode> root;
    std::vector<std::string > tree;
    std::unordered_map<std::string, TokenType> descriptions;
    std:: unordered_map<std::string, TokenType> operators;
    SyntaxAnalyzer(LexicalAnalyzer& lexer, const std::string& outputFilename)
        : lexer(lexer), outputFilename(outputFilename), currentTokenIndex(0)
    {
        tokens = lexer.getTokens();
        previousToken = { TokenType::UNKNOWN, "" }; 
    }

    void parse(bool& parse) 
    {
        try 
        {
            root = parseFunction();
            
            outputParseTree(root, 0, parse); 
        }
        catch (const std::runtime_error& e) 
        {
            std::string sim = "\'";
            std::cerr << "Ошибка: " 
                << "| строка " + std::to_string(currentLine()) + " | " << e.what()
                << " после " + sim + previousToken.value << sim << '\n';
        }
    }


    std::shared_ptr<ParseTreeNode> getParseTree() const
    {
        return root; // Возвращаем корень дерева разбора
    }

    std::vector<std::string> getTree()
    {
        return tree;
    }

    std::unordered_map<std::string, TokenType>  getDescr()
    {
        return descriptions;
    }
    std::unordered_map<std::string, TokenType>& getOp()
    {
        return operators;
    }

private:
    LexicalAnalyzer& lexer;
    std::string outputFilename;
    std::vector<std::pair<Token, int>> tokens; 
    size_t currentTokenIndex;

    std::vector<TokenType> startEnd;
    
    
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
        auto nodeStart = std::make_shared<ParseTreeNode>("Begin");
        auto nodeEnd = std::make_shared<ParseTreeNode>("End");
        auto nodeDop = std::make_shared<ParseTreeNode>("");

        //проверка типа функции
        if (currentToken().type != TokenType::TYPE_INT && currentToken().type != TokenType::TYPE_FLOAT) {
            throw std::runtime_error("Ожидался тип (int или float) ");
        }
       
       
        startEnd.push_back(currentToken().type);
        nextToken(); // TYPE
        
       
        // проверка имени функции
        if (currentToken().type != TokenType::ID) {
            throw std::runtime_error("Ожидалось имя функции ");
        }
        
        tree.push_back("Begin");
        nodeStart->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        tree.push_back(currentToken().value);
        node->children.push_back(nodeStart);
        nextToken(); // FunctionName

        
        if (currentToken().type != TokenType::LPAREN) {
            throw std::runtime_error("Ожидалась '(' ");
        }
        nextToken(); // '('

       
        if (currentToken().type != TokenType::RPAREN) {
            throw std::runtime_error("Ожидалась ')' ");
        }
        nextToken(); // ')'

       
        if (currentToken().type != TokenType::LBRACE) {
            throw std::runtime_error("Ожидалась '{' ");
        }
        nextToken(); // '{'

        // парсим описания
        while (currentToken().type != TokenType::RBRACE)
        {
            if (!findReturn && (currentToken().type == TokenType::TYPE_INT || currentToken().type == TokenType::TYPE_FLOAT))
            {
                
                node->children.push_back(parseDescr()); //////////
            }
            else if (!findReturn &&  currentToken().type == TokenType::ID)
            {
                
                node->children.push_back(parseDescr());
            }
            else if (currentToken().type == TokenType::RETURN) 
            {
                findReturn = true;

                

                tree.push_back("End");
                nodeEnd->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
                tree.push_back(currentToken().value);

                nextToken(); 

                startEnd.push_back(descriptions[currentToken().value]);

                if (startEnd[0] != startEnd[1]) throw std::runtime_error("Тип возвращаемого значения не соответсвует типу функции");

                
               
                if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM
                    || currentToken().type == TokenType::ID)
                {
                    nodeEnd->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
                    tree.push_back(currentToken().value);
                    nextToken(); 
                }

                if (currentToken().type != TokenType::SEMICOLON) 
                {
                    throw std::runtime_error("Ожидалась ';' после return ");
                }
                nextToken(); // ';'

                
            }
            else
            {
                if(findReturn) throw std::runtime_error("Ничего не ожидалось, кроме '}' ");
                else throw std::runtime_error("Ожидалось объявление переменной, присваивание или return ");
            }
        }

        // Проверка на '}'
        if (currentToken().type != TokenType::RBRACE) 
        {
            throw std::runtime_error("Ожидалась '}' ");

        }
        nextToken(); // '}'
        
        
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

    std::vector<std::string> prisvoenie;
    TokenType currentTokenOperator;
    std::string currentValue;

    bool ob = false;
    //парсим Descr
    std::shared_ptr<ParseTreeNode> parseDescr() {
        auto node = std::make_shared<ParseTreeNode>("");

        if (!ob && (currentToken().type == TokenType::TYPE_INT || currentToken().type == TokenType::TYPE_FLOAT)) {
            node = std::make_shared<ParseTreeNode>("Descriptions");
            nextToken();

            

            auto it = descriptions.find(currentToken().value);

            if (it != descriptions.end())
            {
                std::string sim = "\'";
                throw std::runtime_error("Переменная " + sim + currentToken().value + sim + " уже была объявлена");
            }
           
           
            descriptions[currentToken().value] = previousToken.type;

            
               

           //std::cout << previousToken.value << '\n';
            node->children.push_back(parseVarList());

            if (currentToken().type != TokenType::SEMICOLON) {
                throw std::runtime_error("Ожидалась ';' ");
            }
            nextToken(); // ';'
        }
        else if (currentToken().type == TokenType::ID) {
            ob = true;
            node = std::make_shared<ParseTreeNode>("Operators");
            tree.push_back("Operators");
            auto nodeId = std::make_shared<ParseTreeNode>("Id");
            auto nodeOp = std::make_shared<ParseTreeNode>("Op");

            currentTokenOperator = descriptions[currentToken().value];
            currentValue = currentToken().value;

            prisvoenie.push_back(currentToken().value);

            operators[currentToken().value] =  currentToken().type;


            nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            tree.push_back(currentToken().value);
            nodeOp->children.push_back(nodeId);
            nextToken(); // ID

            

            if (currentToken().type == TokenType::ASSIGN) 
            {
                nextToken(); // '='
                nodeOp->children.push_back(parseExpr());

                
                node->children.push_back(nodeOp);
            }
            else {
                throw std::runtime_error("Ожидался оператор '=' ");
            }

            if (currentToken().type != TokenType::SEMICOLON) {
                throw std::runtime_error("Ожидалась ';' ");
            }
            nextToken(); // ';'
        }
        else {
            if (ob) throw std::runtime_error("не ожидалось объявление переменной после присваивание ");
            else throw std::runtime_error("Ожидалось объявление переменной или присваивание ");
        }

        return node;
    }

    std::shared_ptr<ParseTreeNode> parseExpr() 
    {
        auto node = std::make_shared<ParseTreeNode>("Expr");
        auto simpleExprNode = parseSimpleExpr();
        node->children.push_back(simpleExprNode);

        while (currentToken().type == TokenType::PLUS || currentToken().type == TokenType::MINUS) {
            auto opNode = std::make_shared<ParseTreeNode>(currentToken().value);
            
            nextToken(); //  '+' или '-'
           
            node->children.push_back(opNode);
            auto nextSimpleExprNode = parseSimpleExpr();
            
            node->children.push_back(nextSimpleExprNode);
            
            simpleExprNode = nextSimpleExprNode; 
        }

        return node;
    }

    bool itof_ftoi = false;

    std::shared_ptr<ParseTreeNode> parseSimpleExpr()
    {
        auto node = std::make_shared<ParseTreeNode>("SimpleExpr");
        auto nodeId = std::make_shared<ParseTreeNode>("Id");
        auto nodeConst = std::make_shared<ParseTreeNode>("Const");

        if (currentToken().type == TokenType::ID) 
        {
            

            auto it = std::find(prisvoenie.begin(), prisvoenie.end(), currentToken().value);
            std::string znak = "\'";
            if (it == prisvoenie.end())
            {
                throw std::runtime_error("Переменная " + znak + currentToken().value + znak + " не была определена ");
            }
            else if(currentValue == currentToken().value) throw std::runtime_error("Переменная не может быть присвоена сама себе ");
            else
            {
                if (currentTokenOperator != descriptions[currentToken().value] && !itof_ftoi)
                {
                    if(currentTokenOperator == TokenType::TYPE_INT) throw std::runtime_error("Тип данных не соответствует типу переменной 'int'");
                    else throw std::runtime_error("Тип данных не соответствует типу переменной 'float'");
                }
            }
            itof_ftoi = false;

            nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            tree.push_back(currentToken().value);
            node->children.push_back(nodeId);
            nextToken();
        }
        else if (currentToken().type == TokenType::INT_NUM || currentToken().type == TokenType::FLOAT_NUM)
        {
            //std::cout << currentToken().value << '\n';
            operators[currentToken().value] = currentToken().type;

            nodeConst->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            tree.push_back(currentToken().value);
            node->children.push_back(nodeConst);
            nextToken();
        }
        else if (currentToken().type == TokenType::LPAREN) 
        {
            nextToken(); //  '('
            node->children.push_back(parseExpr());
            if (currentToken().type != TokenType::RPAREN) 
            {
                throw std::runtime_error("Ожидалась ')' ");
            }
            nextToken(); //  ')'
        }
        else if (currentToken().type == TokenType::ITOF)
        {
            itof_ftoi = true;




            node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            operators[currentToken().value] = currentToken().type;
            tree.push_back(currentToken().value);
            nextToken(); //  'itof'
            if (currentToken().type != TokenType::LPAREN) 
            {
                throw std::runtime_error("Ожидалась '(' ");
            }
            nextToken(); //  '('

            
            
            if (currentToken().type != TokenType::INT_NUM && currentToken().type != TokenType::ID)
                throw std::runtime_error("Ожидалось целое значение ");

            

            if (currentToken().type == TokenType::ID)
            {
                auto it = std::find(prisvoenie.begin(), prisvoenie.end(), currentToken().value);
                std::string znak = "\'";
                if (it == prisvoenie.end())
                {
                    throw std::runtime_error("Переменная " + znak + currentToken().value + znak + " не была определена ");
                }

                if (descriptions[currentToken().value] != TokenType::TYPE_INT)
                {
                    throw std::runtime_error("Ожидалась переменная с типом 'int' ");
                }
            }

           

            node->children.push_back(parseExpr());
            if (currentToken().type != TokenType::RPAREN) 
            {
                throw std::runtime_error("Ожидалась ')' ");
            }
            
            nextToken(); //  ')'
        }
        else if (currentToken().type == TokenType::FTOI) 
        {
            itof_ftoi = true;
            node->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            operators[currentToken().value] = currentToken().type;
            tree.push_back(currentToken().value);
            nextToken(); //  'ftoi'
            if (currentToken().type != TokenType::LPAREN) 
            {
                throw std::runtime_error("Ожидалась '(' ");
            }
            nextToken(); //  '('
            if (currentToken().type != TokenType::FLOAT_NUM && currentToken().type != TokenType::ID)
                throw std::runtime_error("Ожидалось вещественное значение");

           

            if (currentToken().type == TokenType::ID)
            {
                auto it = std::find(prisvoenie.begin(), prisvoenie.end(), currentToken().value);
                std::string znak = "\'";
                if (it == prisvoenie.end())
                {
                    throw std::runtime_error("Переменная " + znak + currentToken().value + znak + " не была определена ");
                }
                if (descriptions[currentToken().value] != TokenType::TYPE_FLOAT)
                {
                    throw std::runtime_error("Ожидалась переменная с типом 'float' ");
                }
            }




            node->children.push_back(parseExpr());
            if (currentToken().type != TokenType::RPAREN) 
            {
                throw std::runtime_error("Ожидалась ')' ");
            }
            nextToken(); //  ')'
        }
        else 
        {
            throw std::runtime_error("Ожидалось Id, Const, или '(' ");
        }

        return node;
    }


    // Парсим VarList
    std::shared_ptr<ParseTreeNode> parseVarList() 
    {

        
        auto node = std::make_shared<ParseTreeNode>("VarList");
        auto nodeId = std::make_shared<ParseTreeNode>("Id");
       
        if (currentToken().type != TokenType::ID && previousToken.type != TokenType::RETURN) 
        {
            throw std::runtime_error("Ожидалось имя переменной "  );
        }
        nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
        tree.push_back(currentToken().value);
        nextToken(); // ID

        while (currentToken().type == TokenType::COMMA) 
        {
            nextToken(); // ','
            if (currentToken().type != TokenType::ID) 
            {
                
                throw std::runtime_error("Ожидалось имя переменной "   );
            }
            nodeId->children.push_back(std::make_shared<ParseTreeNode>(currentToken().value));
            tree.push_back(currentToken().value);
            nextToken(); // ID
        }

        node->children.push_back(nodeId);

        return node;

       
    }

    //вывод дерева разбора в файл
    void outputParseTree(const std::shared_ptr<ParseTreeNode>& node, int depth, bool& parse) 
    {
        parse = true;
       

        std::ofstream outFile(outputFilename, std::ios::app);
        if (!outFile.is_open())
        {
            std::cerr << "Ошибка: Не удалось открыть файл для записи." << std::endl;
            return;
        }
        outFile << std::string(depth * 2, ' ') << node->value << std::endl; // Отступы
        for (const auto& child : node->children) 
        {
            outputParseTree(child, depth + 1, parse); // Рекурсивный вывод
        }
        outFile.close();


        //for (const auto& child : node->children) {
        //    if (child) { // Проверка на nullptr
        //        std::cout << child->value << std::endl; // Вывод значения узла
        //    }
        //}
        
       
        
    }
};