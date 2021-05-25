/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gss_log_parser.cpp
 * Author: user
 * 
 * Created on 11 de marzo de 2020, 17:51
 */

#include "gss_log_parser.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <fcntl.h>   // open
#include <unistd.h>  // read, write, close

using namespace std;

gss_log_parser::gss_log_parser(const char * config_filename_c_str)
{
    char currentDir[FILENAME_MAX];
    /* get current dir for full minimal dir */
#ifdef __CYGWIN__
    getwd(currentDir);
#else
    getcwd(currentDir, FILENAME_MAX);
#endif
            
    if(config_filename_c_str == NULL)
    {
        config_filename = (string)currentDir + "\\gss_log_parser.ini";
    }
    else
    {
        ostringstream config_filename_ss;
        config_filename_ss << config_filename_c_str;
        config_filename = config_filename_ss.str();
    }
    status = NO_ERROR;
    number_of_lines = 0;
    current_line = 0;
}

int gss_log_parser::parseReport(uint32_t &testsParsedRef)
{
    string rawLogFilename;
    string tcrFilename;
    
    ifstream configFile;
    configFile.open(config_filename);
    if(!configFile.is_open())
    {
        status = CANT_OPEN_CONFIG_FILE;
        return 1;
    }
    if(!getline(configFile, rawLogFilename))
    {
        status = CONFIG_FILE_NOT_VALID;
        configFile.close();
        return 1;
    }
    if(!getline(configFile, tcrFilename))
    {
        status = CONFIG_FILE_NOT_VALID;
        configFile.close();
        return 1;
    }
    configFile.close();
    
    ifstream rawLogFile;
    ofstream tcrFile;
    tcrFile.open(tcrFilename);
    if(!tcrFile.is_open())
    {
        wrong_value = tcrFilename;
        status = CANT_CREATE_TCR_FILE;
        return 1;
    }
    rawLogFile.open(rawLogFilename);
    if(!rawLogFile.is_open())
    {
        wrong_value = rawLogFilename;
        status = CANT_OPEN_LOG_FILE;
        return 1;
    }
    
    current_line = 0;
    number_of_lines = 0;
    string logLine;
    bool headerFound = false;
    bool testInProgress = false;
    while (getline(rawLogFile, logLine))
        ++number_of_lines;
    rawLogFile.clear();
    rawLogFile.seekg(0, ios::beg);
    
    while(getline(rawLogFile, logLine))
    {
        current_line++;
        cout << "\r" << fixed << setprecision(2) << (current_line*100.0)/number_of_lines << "%" <<
                " (" << current_line << "/" << number_of_lines << ")";
        
        if(logLine.substr(0, 5).compare("Test ") == 0)
        {
            if(logLine.substr(logLine.length()-31, 5).compare("began") == 0)
            {
                if(testInProgress == true)
                {
                    /* another test starts but didn't end */
                    tcrFile << "\t\t\t<run bold=\"true\">" << endl;
                    tcrFile << "\t\t\t\t<text>not ended</text>" << endl;
                    tcrFile << "\t\t\t</run>" << endl;
                    tcrFile << "\t\t\t<run>" << endl;
                    tcrFile << "\t\t\t\t<text>.</text>" << endl;
                    tcrFile << "\t\t</evidence>" << endl;
                    tcrFile << "\t\t<status>Fail</status>" << endl;
                    tcrFile << "\t</VTCRTestReport>" << endl;
                }
                tcrFile << "\t<VTCRTestReport test_procedure=\"" <<
                        logLine.substr(5, logLine.length()-37) << "\">" << endl;
                tcrFile << "\t\t<evidence>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text>File: </text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run italics=\"true\">" << endl;
                tcrFile << "\t\t\t\t<text>" << rawLogFilename << "</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text>. Test began at </text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run bold=\"true\">" << endl;
                tcrFile << "\t\t\t\t<text>" << logLine.substr(logLine.length()-24, 24) << "</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text> (line " << current_line << "), </text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                testInProgress = true;
            }
            else if(logLine.substr(5, 5).compare("ended") == 0)
            {
                string ended = "ended";
                string status = "Pass";
                size_t posDate = logLine.find(".") + 2;
                if((logLine.find("not expected packets") != string::npos) ||
                        (logLine.find("error") != string::npos) ||
                        (logLine.find("action check KO") != string::npos))
                {
                    ended += logLine.substr(10, posDate-11);
                    status = "Fail";
                }
                if(ended.size() == 5)//no errors
                {
                    tcrFile << "\t\t\t<run>" << endl;
                    tcrFile << "\t\t\t\t<text>ended</text>" << endl;
                }
                else
                {
                    tcrFile << "\t\t\t<run italics=\"true\" color=\"red\">" << endl;
                    tcrFile << "\t\t\t\t<text>" << ended << "</text>" << endl;
                }
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text> at </text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run bold=\"true\">" << endl;
                tcrFile << "\t\t\t\t<text>" << logLine.substr(posDate, 24) << "</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text> (line " << current_line << ").</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t</evidence>" << endl;
                tcrFile << "\t\t<status>" << status << "</status>" << endl;
                tcrFile << "\t</VTCRTestReport>" << endl;
                testsParsedRef++;
                testInProgress = false;
            }
            else if(logLine.substr(5, 8).compare("canceled") == 0)
            {
                size_t posDate = logLine.find(".") + 2;
                tcrFile << "\t\t\t<run italics=\"true\" color=\"red\">" << endl;
                tcrFile << "\t\t\t\t<text>canceled</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text> at </text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run bold=\"true\">" << endl;
                tcrFile << "\t\t\t\t<text>" << logLine.substr(posDate, 24) << "</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t\t<run>" << endl;
                tcrFile << "\t\t\t\t<text> (line " << current_line << ").</text>" << endl;
                tcrFile << "\t\t\t</run>" << endl;
                tcrFile << "\t\t</evidence>" << endl;
                tcrFile << "\t\t<status>Fail</status>" << endl;
                tcrFile << "\t</VTCRTestReport>" << endl;
                testsParsedRef++;
                testInProgress = false;
            }
            else if((logLine.substr(5, 8).compare("campaign") == 0) &&
                    (!headerFound))
            {
                headerFound = true;
                string name = "";
                string issue = "0";
                string revision = "0";
                string date = "";
                size_t posIss = 0;
                if((posIss = logLine.find(" version")) != string::npos)
                {
                    name = logLine.substr(14, posIss-14);
                    size_t posRev = 0;
                    if((posRev = logLine.substr(posIss+9).find(".")) != string::npos)
                    {
                        issue = logLine.substr(posIss+9, posRev);
                        size_t posDate = 0;
                        if((posDate = logLine.substr(posIss+9+posRev+1).find(".")) != string::npos)
                        {
                            revision = logLine.substr(posIss+9+posRev+1, posDate);
                            size_t posEnd = 0;
                            if((posEnd = logLine.substr(posIss+9+posRev+1+posDate+2).find(" ")) != string::npos)
                            {
                                date = logLine.substr(posIss+9+posRev+1+posDate+2, posEnd);
                            }
                        }
                    }
                }
                tcrFile << "<TestCampaignReport name=\"" << name <<
                        "\" id=\"" << name << "_" << issue << "." << revision <<
                        "\" issue=\"" << issue << "\" revision=\"" << revision <<
                        "\" date=\"" << date << "\">" << endl;
            }
        }
    }
    cout << endl;
    tcrFile << "</TestCampaignReport>" << endl;
    tcrFile.close();
    rawLogFile.close();
    return 0;
}

string gss_log_parser::displayParseReportError()
{
    ostringstream error_ss;
    
    if(errorSection != "")
        error_ss << "At " << errorSection << ": ";
    
    switch(status)
    {
        case CANT_OPEN_CONFIG_FILE:
            error_ss << "Can't open config file \"" << config_filename << "\"";
            break;
            
        case CONFIG_FILE_NOT_VALID:
            error_ss << "Config file \"" << config_filename << "\" is not valid";
            break;
            
        case CANT_CREATE_TCR_FILE:
            error_ss << "Can't create TCR file \"" << wrong_value << "\"";
            break;
            
        case CANT_OPEN_LOG_FILE:
            error_ss << "Can't open log file \"" << wrong_value << "\"";
            break;
            
        default:
            error_ss << "Unknown error " << +status;
            break;
    }
    return error_ss.str();
}