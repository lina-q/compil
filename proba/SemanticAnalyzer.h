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
    std::vector<std::pair<std::string, TokenType>>  operators;

    std::vector<std::string> op;
    
    bool endDes = true;

    void analyzeNode(std::shared_ptr<ParseTreeNode>& root, std::vector<std::string>& tree,
        std::unordered_map<std::string, TokenType>& description, std::vector<std::pair<std::string, TokenType>>& operators) {


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

            TokenType current = description[op[i]];
            std::vector<TokenType> digits;

            for (int j = 0; j < operators.size(); ++j)
            {
                if (operators[j].first == id)
                {
                    ++j;
                    while (j < operators.size() && operators[j].second != TokenType::ID)
                    {
                        if (current == TokenType::TYPE_INT)
                        {
                            if (operators[j].second != TokenType::INT_NUM && operators[j].second != TokenType::FTOI)
                            {
                                throw std::runtime_error("“ип данных не соответствует типу переменной 'int'");
                            }
                            if (operators[j].second == TokenType::FTOI) ++j;
                        }
                        else if (current == TokenType::TYPE_FLOAT)
                        {
                            // »змен€ем оператор || на && дл€ правильной проверки
                            if (operators[j].second != TokenType::FLOAT_NUM && operators[j].second != TokenType::ITOF)
                            {
                                throw std::runtime_error("“ип данных не соответствует типу переменной 'float'");
                            }
                            if (operators[j].second == TokenType::ITOF)
                            {
                                ++j;
                            }
                        }
                        ++j;
                    }
                }
            }

        }

        //проверка на тип данных

        
    }

    
};

//throw std::runtime_error("ѕеременна€ " + sim + operators[i] + sim + " не была объ€влена");