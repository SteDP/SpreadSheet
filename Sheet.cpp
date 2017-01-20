#include "stdafx.h"
#include "Sheet.h"
#include <ctype.h>
#include <stack>
#include <sstream>
#include <math.h>

// ���������� ��������
bool safeAdd(int & a, int b)
{
    // ��������� �������� � 64-������ ����
    long long r = a + b;

    // ��������� ���������
    a += b;

    // ������ ��������� �������� �� ������������
    return r >= INT_MIN && r <= INT_MAX;
}

bool safeMul(int & a, int b)
{
    // ��������� ���������� �������� � 64-������ ����
    long long r = a * b;

    // �������� ����������
    a *= b;

    // ������ ��������� �������� �� ������������
    return r >= INT_MIN && r <= INT_MAX;
}

bool safeDiv(int & a, int b)
{
    // ���� ������ ������ �� ����
    if (b == 0)
        return false; // �� ������ ������

    a /= b; // ����� �������� ����������

    return true; // � ���������� ������
}

bool CSheet::computeCell(int row, int col, std::set<uint> & undefCells)
{
    // ������� ������������� ������
    uint cellId = row * cols + col;

    // ���� ��� ������ ��� ���� ����������
    if (undefCells.count(cellId) != 0)
        return false; // �� ������������ ����������� ������������� �����, �.�. ������

    // �������� ������ �� ����������� ������
    Cell & cell = cells[row][col];

    // ���� ������ ��� ��������� � �������� ������
    if (cell.type == Cell::ErrorCell)
        return false;

    // ���� ������ ��������� � �� �������� ������
    if (cell.type != Cell::ExpressionCell)
        return true;

    // ����� ������ �� ������� � ������
    std::string & expr = cell.symbols;

    // ���� ������ �������� N, �� ��� ������ ���� �������� ������
    if (expr[0] == 'N' && expr.length() == 1)
    {
        cell.type = Cell::ErrorCell;
        cell.symbols = "#EMPTYCEL";

        return false;

    }else if (expr[0] == '\'') // ���� ������ ������ - ��������� �������
    {
        // �������� ���� ������
        expr = expr.substr(1);

        // �������� ������ ����������
        cell.type = Cell::SymbolicCell;

        // ���������� ������
        return true;
    }
    else if (expr[0] == '=' || isdigit(expr[0])) // ���� ������ ������ - ��� ���� "�����" ��� �����
    {
        enum ResultType
        {
            NumericResult,
            SymbolicResult,
            EmptyResult
        } resultType = EmptyResult; // ������� ������ ��������� "������"

        // ���������� ��� �������� ������������� �����������
        std::string  symData;
        int num = 0, numData;
        std::stringstream ss;

        enum ActionType
        {
            ParseNumber,
            ParseIdentifier,
            ParseAll
        } act = ParseAll; // ������� �������� - "������� �����"

        enum SignType
        {
            SignMinus,
            SignPlus,
            SignDiv,
            SignMul
        } lastSign = SignPlus; // ��������� �������� - ����

        // ���� ������ � ��������� ���� �����, �� ��������� ������ � ������ ������
        if (isdigit(expr[0]))
            expr = " " + expr;

        // ����������� �� �������� ���������
        for (auto it = expr.begin() + 1; ; it++)
        {
            // ���� ���������� �� ����� ������ ��� ��������
            if (it == expr.end() || *it == '+' || *it == '-' || *it == '*' || *it == '/') {
                
                bool changeActFlag = true; // ����, ���������, ����� �� � ����� ����� ������ �������� �� "������� �����"

                // �������, ��� �� �������� �������� �������
                switch (act)
                {
                case ParseNumber: // ���� ������� �����
                    if (resultType == EmptyResult) // ���� ��������� ������
                        resultType = NumericResult; // �� ����������, ��� ������ ���������

                    if (resultType == NumericResult) // ���� ��������� ���������
                    {
                        switch (lastSign) // ������� ��������� ����
                        {
                        case SignPlus: // ���� ����
                            if (!safeAdd(num, numData)) // ���� �� ������� ��������� �������
                            {
                                cell.type = Cell::ErrorCell; // ������ ���������� ���������
                                cell.symbols = "#OVERFLOW"; // ������ - ������������
                                return false; // ������ ������
                            }
                            break;
                        case SignMinus: // ���� �����
                            if (!safeAdd(num, -numData)) // ���� �� ������� ��������� �������
                            {
                                cell.type = Cell::ErrorCell; // ������
                                cell.symbols = "#OVERFLOW"; // ������������
                                return false; // �����
                            }
                            break;
                        case SignMul: // ���������
                            if (!safeMul(num, numData)) // ���� �� �������
                            {
                                cell.type = Cell::ErrorCell; // ������
                                cell.symbols = "#OVERFLOW"; // ������������
                                return false; // �����
                            }
                            break;
                        case SignDiv: // �������
                            if (!safeDiv(num, numData)) // ���� �� �������
                            {
                                cell.type = Cell::ErrorCell; // ������
                                cell.symbols = "#OVERFLOW"; // ������������
                                return false; // �����
                            }
                            break;
                        }
                    }
                    else { // ���� �������� ����������
                        switch (lastSign)
                        {
                        case SignPlus: // ��������� ���� - ����
                            ss << numData; // ��������� � ������ ��������� �����
                            break;
                        default: // ���� ��������� ���� �����-�� ������
                            cell.type = Cell::ErrorCell; // ������
                            cell.symbols = "#WRONGOPR"; // �������� ��������
                            return false; // �����
                        }
                    }
                    break;
                case ParseIdentifier: // ���� ������� �������������
                {
                    // ��������� ������� ������ � �������
                    int tC = tolower(symData[0]) - 'a', tR = numData - 1; 

                    // ���� ������� ����� �� �������
                    if (tC < 0 || tC >= cols)
                    {
                        cell.type = Cell::ErrorCell; // ������
                        cell.symbols = "#WRONGCOL"; // �������� �������
                        return false; // �����
                    }
                    // ���� ������ ����� �� �������
                    if (tR < 0 || tR >= rows)
                    {
                        cell.type = Cell::ErrorCell; // ������
                        cell.symbols = "#WRONGROW"; // �������� ������
                        return false; // �����
                    }
                    // ���� �������
                    undefCells.insert(cellId); // ������ � "�������������� ������" ������������� �������
                    if (!computeCell(tR, tC, undefCells)) // ���� �� ������� ��������� ������� ������
                    {
                        cell.type = Cell::ErrorCell; // ������
                        cell.symbols = "#UNDEFIND"; // �������������� ���������
                        undefCells.erase(cellId); // ������� �������������
                        return false; // �����
                    }
                    undefCells.erase(cellId); // ������� �������������
                    Cell & tCell = cells[tR][tC]; // �������� ������ �� ������� ������

                    switch (tCell.type) // ���������� ��� ������� ������
                    {
                    case Cell::NumericCell: // ���� ��������
                        changeActFlag = false; // �� ������ �������� �� "������� ��"
                        act = ParseNumber; // ������ �������� �� "������� �����"
                        numData = tCell.number; // ���������� ����� �� ������� ������
                        it -= 1; // ������������ �� ���� ������ �����
                        break;
                    case Cell::SymbolicCell: // ���� ����������� ���
                        if (resultType == EmptyResult) // ���� ��������� ��� ������
                            resultType == SymbolicResult; // ������ �� ������ ����������
                        else if (resultType == NumericResult) // ���� ��������� ��� ��������
                        {
                            resultType = SymbolicResult; // ������ ����������

                            ss << num; // ������ ����� � ������
                        }

                        ss << tCell.symbols; // ���������� � ������ ������� �� ������� ������

                    }
                    break;
                }
                }
                if (it == expr.end()) // ���� ��� ����� ������
                {
                    if (resultType == NumericResult) // ���� ��������� ���������
                    {
                        cell.type = Cell::NumericCell; // �������� ������ ���������
                        cell.number = num; // ��������� - �����
                    }
                    else if (resultType == SymbolicResult) { // ���� ��������� ����������
                        cell.type = Cell::SymbolicCell; // ������ ����������
                        cell.symbols = ss.str(); // ��������� - ������
                    }
                    else { // ���� ������ ���������
                        cell.type = Cell::ErrorCell; // ������
                        cell.symbols = "#EMPTYCEL"; // ������ ������
                        return false; // ����� � �������
                    }
                    return true; // ����� � ������������� �����������
                }
                switch (*it) // ���� �� ����� ������, �� ��������� ������� ����
                {
                case '+': // ����
                    lastSign = SignPlus; // ����������
                    break;
                case '-': // �����
                    lastSign = SignMinus; // ����������
                    break;
                case '*': // ���������
                    lastSign = SignMul; // ����������
                    break;
                case '/': // �������
                    lastSign = SignDiv; // ����������
                    break;
                }

                if(changeActFlag) // ���� ���� �������� ��������
                    act = ParseAll; // ���������� �� "������� ��"
            }
            else if (isdigit(*it)) // ���� ����������� �����
            {
                switch(act) // ������� ��������
                {
                    case ParseAll: // ������� ��
                        numData = *it - '0'; // ������������� ����� ����� ������ �����
                        act = ParseNumber; // ������� �����
                        break;
                    case ParseNumber: // ������� �����
                    case ParseIdentifier: // ��� �������������
                        if (INT_MAX / 10 <= numData) // ���� ������ � ������� ����
                        {
                            cell.type = Cell::ErrorCell; // ������
                            cell.symbols = "#OVERFLOW"; // ������������
                            return false; // �����
                        }
                        numData = numData * 10 + (*it - '0'); // � �������������� ��������� �����
                        break;
                }
            } else if (isalpha(*it)) // ���� ��������� ������ ��������
            {
                switch (act) // ������� ��������
                {
                case ParseAll: // ������� ��
                    symData = *it; // ������������� ������ ����� ����� �������
                    act = ParseIdentifier; // ������� �������������
                    numData = 0; // ������������� ����� ����� ����
                    break;
                case ParseNumber: // ������� �����
                    cell.type = Cell::ErrorCell; // ������
                    cell.symbols = "#WRONGCHR"; // �������� ������
                    return false; // �����
                case ParseIdentifier: // ������� �������������
                    cell.type = Cell::ErrorCell; // ������
                    cell.symbols = "#WRONGCOL"; // �������� �������
                    return false; // �����
                }
            } else { // ���� ���������� ���������������� ������
                cell.type = Cell::ErrorCell; // ������
                cell.symbols = "#WRONGCHR"; // �������� ������
                return false; // �����
            }
        }
    }
        
    return false; // ����� � �������
}


CSheet::CSheet(int r, int c)
{
    rows = r; // ���������� ���������� �����
    cols = c; // ��������

    cells.resize(r); // ����������� ��������� ����� �� ���������� �����

    for (auto it = cells.begin(); it != cells.end(); it++) // ����� ��������� �� ������ ������
        it->resize(c); // ����������� �� ���������� ��������
}

CSheet::~CSheet()
{
}

void CSheet::addSource(const std::string & source) // �������� ��������
{
    if (sources.size() < rows * cols) // ���� ����� �� �������
        sources.push_back(source); // ���������
}

void CSheet::computeSheet()
{

    int r = 0, c = 0; // �������� ��� ������� ������ � �������

    for (auto s : sources) // ����������� �� ����������
    {
        cells[r][c].set(Cell::ExpressionCell, s); // ������ ��� ������� ������ ������� ���������

        c += 1; // ��������� � ���������� �������
        r += c / cols; // ���� ����, ��������� � ��������� ������
        c %= cols; // ����� ������� ������ ������ ��������� ��������
    }

    for (r = 0; r < rows; r++) // ����������� �� �������
    {
        for (c = 0; c < cols; c++) // ����������� �� ��������
        {
            if (cells[r][c].type == Cell::ExpressionCell) // ���� � ������ ������ ���������
                computeCell(r, c); // �������� ���
        }
    }

}

std::string CSheet::getResult()
{
    std::stringstream ss; // ��������� ����� ��� ������������ ����������

    for (int r = 0; r < rows; r++) // ����������� �� �������
    {
        for (int c = 0; c < cols; c++) // �� ��������
        {
            ss.width(10); // ������ - 10
            if (cells[r][c].type == Cell::NumericCell) // ���� ������ ���������
                ss << cells[r][c].number; // ������� �����
            else // �����
                ss << cells[r][c].symbols; // �������
        }
        ss << std::endl; // � ����� ������ ������� �� ��������� ������
    }
    
    return ss.str(); // ������ �������������� ������
}
