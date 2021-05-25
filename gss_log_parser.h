/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gss_log_parser.h
 * Author: user
 *
 * Created on 11 de marzo de 2020, 17:51
 */

#ifndef GSS_LOG_PARSER_H
#define GSS_LOG_PARSER_H

#include <stdint.h>
#include <string>

class gss_log_parser {
public:
    gss_log_parser(const char * config_filename_c_str);
    int parseReport(uint32_t &testsParsedRef);
    std::string displayParseReportError();
    
private:
    enum Status : uint32_t {
        NO_ERROR,
        CANT_OPEN_CONFIG_FILE,
        CONFIG_FILE_NOT_VALID,
        CANT_CREATE_TCR_FILE,
        CANT_OPEN_LOG_FILE
    } status;
    uint32_t number_of_lines;
    uint32_t current_line;
    std::string config_filename;
    
    std::string errorSection;
    std::string wrong_value;
};

#endif /* GSS_LOG_PARSER_H */

