/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: user
 *
 * Created on 11 de marzo de 2020, 17:10
 */

#include <iostream>
#include "gss_log_parser.h"

using namespace std;

int main(int argc, char** argv) {

    int status = 0;
    uint32_t testsParsed = 0;
    
    gss_log_parser parser(argv[1]);
    if((status = parser.parseReport(testsParsed)) != 0)
    {
        cout << endl << parser.displayParseReportError() << endl;
        getchar();
        return 1;
    }
    cout << endl << "Parsed " << testsParsed << " tests" << endl;
    getchar();
    return status;
}


