#include <iostream>
#include <string>
#include "Token.h"
#include "LexicalAnalyzer.h"
#include "HashTable.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"
#include <fstream>

int main()
{
    setlocale(LC_ALL, "ru");
    std::string filename = "input.txt";
    std::string outputFilename = "output.txt"; // ��� ����� ��� ������ ������ �������

    HashTable hashTable;
    LexicalAnalyzer lexicalAnalyzer(filename, hashTable);
    lexicalAnalyzer.tokenize(); 

    hashTable.print(); 

    bool parse = false;

  
    SyntaxAnalyzer syntaxAnalyzer(lexicalAnalyzer, outputFilename);
     syntaxAnalyzer.parse(parse);

     
    
    
    if(parse)
    {
        SemanticAnalyzer semanticAnalyzer(syntaxAnalyzer, outputFilename);
        semanticAnalyzer.analyze();
    }

    return 0;
}