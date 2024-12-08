#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "LexicalAnalyzer.h"
#include "HashTable.h"
#include "SyntaxAnalyzer.h"
#include <utility>
#include <unordered_map>
#include <stdexcept>



class SemanticAnalyzer
{
public:
    SemanticAnalyzer(SyntaxAnalyzer& syntaxAnalyzer, const std::string& outputFilename)
        : root(syntaxAnalyzer.getParseTree()),
        tree(syntaxAnalyzer.getTree()), 
        startEnd(syntaxAnalyzer.getStartEnd()),
        description(syntaxAnalyzer.getDescr()),
        outputFilename(outputFilename),
        operators(syntaxAnalyzer.getOp())
        
    {}

    void analyze() 
    {
        try 
        {
            analyzeNode(root, tree, description, operators);

            outputPostForm(root, 0);
            

            outPut(startEnd);
        }
        catch (const std::runtime_error& e)
        {
           
            std::cerr << "Ошибка: " << e.what() << '\n';
        }
        
    }

private:
    std::shared_ptr<ParseTreeNode> root;
    std::vector<std::string> tree;
    std::unordered_map<std::string, std::string> variableTypes; 
    std::string returnType; 
    std::unordered_map<std::string, TokenType>  description;
    std::unordered_map<std::string, TokenType>& operators;
    std::vector<TokenType> startEnd;
    std::string outputFilename;

    std::vector<std::string> op;
    
    bool endDes = true;

    void analyzeNode(std::shared_ptr<ParseTreeNode>& root, std::vector<std::string>& tree,
        std::unordered_map<std::string, TokenType>& description, std::unordered_map<std::string, TokenType>& operators) {


        for (int i = 0; i < tree.size(); ++i)
        {
            //outFile << tree[i] << '\n';
            if (tree[i] == "Operators")
            {
                endDes = false;
                op.push_back(tree[i + 1]);

            }
        }

        

        //for (int i = 0; i < operators.size(); ++i) outFile << operators[i].first << '\n';




        for (int i = 0; i < op.size(); ++i)
        {
            std::string id = op[i];
            auto it = description.find(id);
            if (it == description.end())
            {
                std::string sim = "\'";
                throw std::runtime_error("Переменная " + sim + id + sim + " не была объявлена");
            }

            
            std::vector<TokenType> digits;

            for (int j = 0; j < tree.size(); ++j)
            {
                if (tree[j] == "End") break;
                TokenType current = description[op[i]];
                if (tree[j] == "Operators")
                {
                    ++j;
                    ++j;
                    while (tree[j] != "Operators")
                    {
                        if(current == TokenType::TYPE_INT)
                        {
                            
                            if (operators[tree[j]] != TokenType::INT_NUM && operators[tree[j]] != TokenType::FTOI && operators[tree[j]] != TokenType::ID)
                            {
                                std::string znak = "\'";
                                throw std::runtime_error("Тип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'int'");
                            }
                            if (operators[tree[j]] == TokenType::FTOI)
                            {
                                ++j;
                                ++j;
                            }
                            if (operators[tree[j]] == TokenType::ID)
                            {
                                if (description[tree[j]] != current)
                                {
                                    std::string znak = "\'";
                                    throw std::runtime_error("Тип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'int'");
                                }
                                else ++j;
                            }
                            if (operators[tree[j]] == TokenType::INT_NUM)
                            {
                                ++j;
                            }
                            if (tree[j] == "End") break;
                        }
                        else
                        {
                           
                            if (operators[tree[j]] != TokenType::FLOAT_NUM && operators[tree[j]] != TokenType::ITOF && operators[tree[j]] != TokenType::ID)
                            {
                                std::string znak = "\'";
                                throw std::runtime_error("Тип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'float'");
                            }
                            if (operators[tree[j]] == TokenType::ITOF)
                            {
                                ++j;
                                ++j;
                            }
                            if (operators[tree[j]] == TokenType::ID)
                            {
                                if (description[tree[j]] != current)
                                {
                                    std::string znak = "\'";
                                    throw std::runtime_error("Тип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'float'");
                                }
                                else ++j;
                            }
                            if (operators[tree[j]] == TokenType::FLOAT_NUM)
                            {
                                ++j;
                            }
                            if (tree[j] == "End") break;
                            
                        }
                        
                    }
                    --j;
                    ++i;

                        

                }
               
               
            }

            
            

        }

        //проверка на тип данных

        
    }

    std::vector<std::string> postfix;

    void outputPostForm(const std::shared_ptr<ParseTreeNode>& node, int depth)
    {

        //outFile << std::string(depth * 2, ' ') << node->value << std::endl; // Отступы
        for (const auto& child : node->children)
        {
            if (child->value == "Descriptions") postfix.push_back(child->value);
            if (child->value == "Op") postfix.push_back(child->value);
            //if (child->value == "Operators") postfix.push_back("=");
            outputPostForm(child, depth + 1); // Рекурсивный вывод
        }

        for (const auto& child : node->children) {
            //if (child) { // Проверка на nullptr
            //    outFile << child->value << std::endl; // Вывод значения узла
            //}
            if (child->value != "Id" && child->value != "VarList" && child->value != "Const" && child->value != "SimpleExpr" && child->value != "Expr"
                && child->value != "Operators" && child->value != "Descriptions" &&
                child->value != "Begin" && child->value != "End" && child->value != "Op")
            {
                postfix.push_back(child->value);
            }
        }

        

    }

    bool descr = false;
    bool first = false;
    bool ret = false;
   

    void outPut(std::vector<TokenType> startEnd)
    {

        std::ofstream outFile(outputFilename, std::ios::app);

        outFile << '\n';
        outFile << '\n';
        outFile << '\n';

        int i =0 ;
        for (int i = 0; i < postfix.size(); ++i)
        {
            if (postfix[i] == "main")
            {
                if (startEnd[0] == TokenType::TYPE_INT)
                    outFile << "int " << postfix[i] << " DECLFUNC";
                else outFile << "float " << postfix[i] << " DECLFUNC";
                ++i;
            }
            if (postfix[i] == "Descriptions")
            {
                descr = true;
                outFile << '\n';
                ++i;
                if (description[postfix[i]] == TokenType::TYPE_INT) outFile << "int ";
                if (description[postfix[i]] == TokenType::TYPE_FLOAT) outFile << "float ";
                

            }
            if (postfix[i] == "Op")
            {
                
                ++i;
                if(first) outFile << " =";
                outFile << '\n';
                first = true;
                

            }

            if (postfix[i] == "return")
            {
                outFile << " =";

                outFile << '\n';
                outFile << postfix[i+1] << " " << postfix[i];

                ++i;
                ret = true;
                
            }

            if (descr)
            {
                outFile << postfix[i] << " DECL";
                descr = false;
            }
            else if (ret) break;
            
            else outFile << postfix[i] << " ";

            
        }

        outFile << '\n';

        outFile.close();
    }

    

    
};

