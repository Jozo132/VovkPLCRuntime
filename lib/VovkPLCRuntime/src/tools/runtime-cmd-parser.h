// runtime-cmd-parser.h - 1.0.0 - 2022-12-11
//
// Copyright (c) 2022 J.Vovk
//
// This file is part of VovkPLCRuntime.
//
// VovkPLCRuntime is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VovkPLCRuntime is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once


#define __RUNTIME_COMMAND_PARSER_MAX_COMMAND_LENGTH 32
#define __RUNTIME_COMMAND_PARSER_MAX_DATA_LENGTH 1024

class RuntimeCommandParser {

    typedef struct { char* command; int offset; int size; int checksum; uint8_t* data; } ParsedData;
    ParsedData parsedData;

    bool parseCommand(const char* input, int& index) {
        int i = index;
        int commandLength = 0;
        while (input[i] != ',' && input[i] != '\0') {
            commandLength++;
            i++;
        }
        if (commandLength > __RUNTIME_COMMAND_PARSER_MAX_COMMAND_LENGTH)
            return true;

        // parsedData.command = (char*) malloc(commandLength + 1);
        parsedData.command = new char[commandLength + 1];
        i = 0;
        while (input[index] != ',' && input[index] != '\0') {
            parsedData.command[i] = input[index];
            index++;
            i++;
        }
        parsedData.command[i] = '\0';
        return false;
    }

    bool parseSize(const char* input, int& index) {
        int i = index;
        int sizeLength = 0;
        while (input[i] >= '0' && input[i] <= '9') {
            sizeLength++;
            i++;
        }
        if (sizeLength == 0 || sizeLength > 7)
            return true;

        parsedData.size = 0;
        for (i = index; i < index + sizeLength; i++)
            parsedData.size = parsedData.size * 10 + (input[i] - '0');

        index += sizeLength;

        if (parsedData.size % 2 != 0)
            return true;

        if (parsedData.size > __RUNTIME_COMMAND_PARSER_MAX_DATA_LENGTH)
            return true;

        return false;
    }

    bool parseOffset(const char* input, int& index) {
        int i = index;
        int offsetLength = 0;
        while (input[i] >= '0' && input[i] <= '9') {
            offsetLength++;
            i++;
        }
        if (offsetLength == 0 || offsetLength > 7)
            return true;

        parsedData.offset = 0;
        for (i = index; i < index + offsetLength; i++)
            parsedData.offset = parsedData.offset * 10 + (input[i] - '0');

        index += offsetLength;
        return false;
    }

    bool parseChecksum(const char* input, int& index) {
        int i = index;
        int sizeLength = 0;
        while (input[i] >= '0' && input[i] <= '9') {
            sizeLength++;
            i++;
        }
        if (sizeLength == 0 || sizeLength > 7)
            return true;

        parsedData.checksum = 0;
        for (i = index; i < index + sizeLength; i++)
            parsedData.checksum = parsedData.checksum * 10 + (input[i] - '0');

        index += sizeLength;
        return false;
    }

    bool parseData(const char* input, int& index, bool checkSize = true) {
        int dataLength = 0;
        int temp_i = index;
        while ((input[temp_i] >= '0' && input[temp_i] <= '9') || (input[temp_i] >= 'A' && input[temp_i] <= 'F') || (input[temp_i] >= 'a' && input[temp_i] <= 'f')) {
            dataLength++;
            temp_i++;
        }

        if (checkSize && (dataLength != (parsedData.size * 2)))
            return true;

        if (dataLength > __RUNTIME_COMMAND_PARSER_MAX_DATA_LENGTH)
            return true;

        // parsedData.data = (uint8_t*) malloc(dataLength);
        parsedData.data = new uint8_t[dataLength];

        int j = 0;
        while (input[index] != '\0') {
            uint8_t hi = input[index] - '0';
            if (hi > ('a' - '0' - 10)) hi -= 'a' - '0' - 10;
            if (hi > ('A' - '0' - 10)) hi -= 'A' - '0' - 10;
            uint8_t lo = input[index + 1] - '0';
            if (lo > ('a' - '0' - 10)) lo -= 'a' - '0' - 10;
            if (lo > ('A' - '0' - 10)) lo -= 'A' - '0' - 10;
            parsedData.data[j] = (hi << 4) | lo;
            index += 2;
            j++;
        }
        return false;
    }

    int calculateChecksum(uint8_t* data, int size) {
        int checksum = 0;
        for (int i = 0; i < size; i++)
            checksum += data[i];
        return checksum;
    }

    void (*callRun)(void);
    void (*callPause)(void);
    void (*callStop)(void);
    void (*callDebug)(void);
    void (*callReset)(void);
    void (*callStep)(void);

    void (*callProgramUpload)(uint8_t* data, int size);
    void (*callProgramDownload)(int size);
    void (*callProgramErase)(void);
    void (*callProgramResize)(int size);

    // Parse command input string
    // Example: "RUN\n" -> will call 'callRun()'
    // Example: "PAUSE\n" -> will call 'callPause()'
    // Example: "STOP\n" -> will call 'callStop()'
    // Example: "DEBUG\n" -> will call 'callDebug()'
    // Example: "RESET\n" -> will call 'callReset()'
    // Example: "STEP\n" -> will call 'callStep()'
    // Example: "PROGRAM_UPLOAD,4,10,01020304\n" -> will call 'callProgramUpload(uint8_t[0x01, 0x02, 0x03, 0x04], 4, 10)'
    // Example: "PROGRAM_DOWNLOAD,4\n" -> will call 'callProgramDownload(4)'
    // Example: "PROGRAM_ERASE\n" -> will call 'callProgramErase()'
    // Example: "PROGRAM_RESIZE,4\n" -> will call 'callProgramResize(4)'


    enum Command {
        UNKNOWN = 0,
        RUN,
        PAUSE,
        STOP,
        DEBUG,
        RESET,
        STEP,
        PROGRAM_UPLOAD,
        PROGRAM_DOWNLOAD,
        PROGRAM_ERASE,
        PROGRAM_RESIZE,
    };

    bool handle_run(const char* inputString, int& index) {
        if (callRun == nullptr) return true;
        callRun();
        return false;
    }

    bool handle_pause(const char* inputString, int& index) {
        if (callPause == nullptr) return true;
        callPause();
        return false;
    }

    bool handle_stop(const char* inputString, int& index) {
        if (callStop == nullptr) return true;
        callStop();
        return false;
    }

    bool handle_debug(const char* inputString, int& index) {
        if (callDebug == nullptr) return true;
        callDebug();
        return false;
    }

    bool handle_reset(const char* inputString, int& index) {
        if (callReset == nullptr) return true;
        callReset();
        return false;
    }

    bool handle_step(const char* inputString, int& index) {
        if (callStep == nullptr) return true;
        callStep();
        return false;
    }

    bool handle_program_upload(const char* inputString, int& index) {
        if (callProgramUpload == nullptr) return true;
        if (parseSize(inputString, index)) return true;
        if (parseChecksum(inputString, index)) return true;
        if (parseData(inputString, index)) return true;
        int checksum = calculateChecksum(parsedData.data, parsedData.size);
        if (parsedData.checksum != checksum) return true;
        callProgramUpload(parsedData.data, parsedData.size);
        return false;
    }

    bool handle_program_download(const char* inputString, int& index) {
        if (callProgramDownload == nullptr) return true;
        if (parseSize(inputString, index)) return true;
        callProgramDownload(parsedData.size);
        return false;
    }

    bool handle_program_erase(const char* inputString, int& index) {
        if (callProgramErase == nullptr) return true;
        callProgramErase();
        return false;
    }

    bool handle_program_resize(const char* inputString, int& index) {
        if (callProgramResize == nullptr) return true;
        if (parseSize(inputString, index)) return true;
        callProgramResize(parsedData.size);
        return false;
    }

    bool handle_unknown(const char* inputString, int& index) {
        if (!Serial) return true;
        Serial.print(F("RuntimeCommandParser - Unknown command: \""));
        Serial.print(inputString);
        Serial.println("\"");
        return true;
    }


    int decodeCommand() {
        if (strcmp(parsedData.command, "RUN") == 0) return RUN;
        if (strcmp(parsedData.command, "PAUSE") == 0) return PAUSE;
        if (strcmp(parsedData.command, "STOP") == 0) return STOP;
        if (strcmp(parsedData.command, "RESET") == 0) return RESET;
        if (strcmp(parsedData.command, "DEBUG") == 0) return DEBUG;
        if (strcmp(parsedData.command, "STEP") == 0) return STEP;
        if (strcmp(parsedData.command, "PROGRAM_UPLOAD") == 0) return PROGRAM_UPLOAD;
        if (strcmp(parsedData.command, "PROGRAM_DOWNLOAD") == 0) return PROGRAM_DOWNLOAD;
        if (strcmp(parsedData.command, "PROGRAM_ERASE") == 0) return PROGRAM_ERASE;
        if (strcmp(parsedData.command, "PROGRAM_RESIZE") == 0) return PROGRAM_RESIZE;
        return UNKNOWN;
    }

    uint8_t parseInput(const char* inputString) {
        memset(&parsedData, 0, sizeof(parsedData));
        int i = 0;
        bool problem = false;

        // Parse command
        problem = parseCommand(inputString, i);
        if (problem) return 1;

        int command = decodeCommand();
        if (command == UNKNOWN) return handle_unknown(inputString, i);
        if (command == RUN) return handle_run(inputString, i);
        if (command == PAUSE) return handle_pause(inputString, i);
        if (command == STOP) return handle_stop(inputString, i);
        if (command == RESET) return handle_reset(inputString, i);
        if (command == DEBUG) return handle_debug(inputString, i);
        if (command == STEP) return handle_step(inputString, i);
        if (command == PROGRAM_UPLOAD) return handle_program_upload(inputString, i);
        if (command == PROGRAM_DOWNLOAD) return handle_program_download(inputString, i);
        if (command == PROGRAM_ERASE) return handle_program_erase(inputString, i);
        if (command == PROGRAM_RESIZE) return handle_program_resize(inputString, i);
        return 0;
    }

};