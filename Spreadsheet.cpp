// Spreadsheet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include "Sheet.h"

int main()
{
    int rows, cols;
    
    // ������ ���������� ����� � ��������
    std::cin >> rows >> cols;
    
    // ��������� ������� ������� �������
    CSheet sheet(rows, cols);

    // ���� � ����� ������ �����
    std::cin.seekg(std::ios::end);

    int cells = rows*cols;

    do
    {
        std::string str;

        std::cin >> str;

        // ��������� ��������� ������ � �������
        sheet.addSource(str);

    } while (--cells); // � ���, ���� �� ������� ��� ������

    // ����������� �������
    sheet.computeSheet();

    // ���������� ���� �� ��� ������
    std::cout << "\n\n\n";

    // ������� �������������� �������
    std::cout << sheet.getResult();
	std::cin>>rows;
    return 0;
}

