// Spreadsheet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "Sheet.h"

int main()
{
    int rows, cols;
    
    // читаем количество строк и столбцов
    std::cin >> rows >> cols;
    
    // формируем таблицу нужного размера
    CSheet sheet(rows, cols);

    // идет в конец потока ввода
    std::cin.seekg(std::ios::end);

    int cells = rows*cols;

    do
    {
        std::string str;

        std::cin >> str;

        // добавл€ем считанные данные в таблицу
        sheet.addSource(str);

    } while (--cells); // и так, пока не считаем все €чейки

    // обсчитываем таблицу
    sheet.computeSheet();

    // опускаемс€ ниже на три строки
    std::cout << "\n\n\n";

    // выводим результирующую таблицу
    std::cout << sheet.getResult();
	std::cin>>rows;
    return 0;
}

