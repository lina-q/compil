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
    SemanticAnalyzer(SyntaxAnalyzer& syntaxAnalyzer)
        : root(syntaxAnalyzer.getParseTree()),
        tree(syntaxAnalyzer.getTree()), 
        description(syntaxAnalyzer.getDescr()), 
        operators(syntaxAnalyzer.getOp())
        
    {}

    void analyze() 
    {
        try 
        {
            analyzeNode(root, tree, description, operators);
        }
        catch (const std::runtime_error& e)
        {
           
            std::cerr << "ќшибка: " << e.what() << '\n';
        }
        
    }

private:
    std::shared_ptr<ParseTreeNode> root;
    std::vector<std::string> tree;
    std::unordered_map<std::string, std::string> variableTypes; 
    std::string returnType; 
    std::unordered_map<std::string, TokenType>  description;
    std::unordered_map<std::string, TokenType>& operators;

    std::vector<std::string> op;
    
    bool endDes = true;

    void analyzeNode(std::shared_ptr<ParseTreeNode>& root, std::vector<std::string>& tree,
        std::unordered_map<std::string, TokenType>& description, std::unordered_map<std::string, TokenType>& operators) {


        for (int i = 0; i < tree.size(); ++i)
        {
            //std::cout << tree[i] << '\n';
            if (tree[i] == "Operators")
            {
                endDes = false;
                op.push_back(tree[i + 1]);

            }
        }

        

        //for (int i = 0; i < operators.size(); ++i) std::cout << operators[i].first << '\n';




        for (int i = 0; i < op.size(); ++i)
        {
            std::string id = op[i];
            auto it = description.find(id);
            if (it == description.end())
            {
                std::string sim = "\'";
                throw std::runtime_error("ѕеременна€ " + sim + id + sim + " не была объ€влена");
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
                                throw std::runtime_error("“ип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'int'");
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
                                    throw std::runtime_error("“ип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'int'");
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
                                throw std::runtime_error("“ип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'float'");
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
                                    throw std::runtime_error("“ип данных " + znak + tree[j] + znak + " не соответствует типу переменной 'float'");
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

    
};

//throw std::runtime_error("ѕеременна€ " + sim + operators[i] + sim + " не была объ€влена");