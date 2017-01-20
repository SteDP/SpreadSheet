#pragma once

#include <string>
#include <vector>
#include <utility>
#include <set>

typedef unsigned int uint;

class CSheet
{
private:
    class Cell // ���, ����������� ������ ������
    {
    public:
        // ���, ����������� ��� ���������� ������
        enum CellType
        {
            NumericCell,
            SymbolicCell,
            ErrorCell,
            ExpressionCell
        };

        // ����������, �������� ��� ������
        CellType type;

        std::string symbols;
        int number;

        // ��������� �����������
        Cell()
        {
            type = ErrorCell;
            symbols = "#EMPTYCEL";
        }

        ~Cell()
        {

        }

        Cell(const Cell &o)
        {
            type = o.type;
            if (type == NumericCell)
                number = o.number;
            else
                symbols = o.symbols;
        }

        void set(int num)
        {
            type = NumericCell;
            number = num;
        }

        void set(CellType t, const std::string &v)
        {
            type = t;
            symbols = v;
        }
    };

    // ��� ������ ������ �������
    std::vector<std::vector<Cell>> cells;

    // ��������� ������ �������
    std::vector<std::string> sources;

    int rows, cols;

    /* 
        ��������� �������� ������
        undefCells - ������, �������� �������������� ���� �����, ������� ���� ����������
    */
    bool computeCell(int row, int col, std::set<uint> & undefCells = std::set<uint>());

public:
    CSheet(int rows, int cols);
    ~CSheet();

    // �������� �������� ��� ��������� ������
    void addSource(const std::string &source);

    // ��������� �������� ��������� ��� �������
    void computeSheet();

    // �������� ��������� ������������� ��� �������
    std::string getResult();
};

