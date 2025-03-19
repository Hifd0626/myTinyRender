#pragma once
#include <vector>
#include <cmath>
#include <iostream>
struct Matrix
{
    std::vector<std::vector<double>> m;
    int rows,cols;
    Matrix(int rows, int cols): rows(rows), cols(cols){
        m.resize(rows);
        for(int i=0;i<rows;i++){
            m[i].resize(cols);
        }
    }
    void identity(){
        for(int i=0;i<rows;i++){
            for(int j=0;j<cols;j++){
                m[i][j] = (i==j)?1:0;
            }
        }
    }
    Matrix operator*(const Matrix& other) const {
        Matrix result(rows, other.cols);
        for(int i=0;i<rows;i++){
            for(int j=0;j<other.cols;j++){
                result.m[i][j] = 0;
                for(int k=0;k<cols;k++){
                    result.m[i][j] += m[i][k]*other.m[k][j];
                }
            }
        }
        return result;
    }

    double* operator[](int i){
        return &m[i][0];
    }
    void printMatrix(){
        for(int i=0;i<rows;i++){
            for(int j=0;j<cols;j++){
                std::cout<<m[i][j]<<" ";
            }
            std::cout<<std::endl;
        }
    }
    

};
