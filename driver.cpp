/*
 * @authors Josh Tittiranonda, Surya Duraivenkatesh
 * @date: 03/2025
 * Welcome to OHS Y86-64 Emulator in C++! This is a C++ program to help emulate the processing of an object file into binary by a compiler. It takes in
 an assembly file as input and outputs the binary file.
 */

#include <bits/stdc++.h>
using namespace std;

#define MAX_MEMORY 100000

vector<vector<string>> tokenized_lines;
unordered_map<string, int> symbolic_names;

int memory[MAX_MEMORY];
int program_counter;
vector<int> instruction_starts;
vector<int> instruction_stops;

void _init();
void assemble();

int main()
{
    _init();

    cout << "  ______   __    __   ______         __      __   ______    ______  \n";
    cout << " /      \\ |  \\  |  \\ /      \\       |  \\    /  \\ /      \\  /      \\ \n";
    cout << "|  $$$$$$\\| $$  | $$|  $$$$$$\\       \\$$\\  /  $$|  $$$$$$\\|  $$$$$$\\\n";
    cout << "| $$  | $$| $$__| $$| $$___\\$$        \\$$\\/  $$ | $$__/ $$| $$___\\$$\n";
    cout << "| $$  | $$| $$    $$ \\$$    \\          \\$$  $$   >$$    $$| $$    \\ \n";
    cout << "| $$  | $$| $$$$$$$$ _\\$$$$$$\\          \\$$$$   |  $$$$$$ | $$$$$$$\\\n";
    cout << "| $$__/ $$| $$  | $$|  \\__| $$          | $$    | $$__/ $$| $$__/ $$\n";
    cout << " \\$$    $$| $$  | $$ \\$$    $$          | $$     \\$$    $$ \\$$    $$\n";
    cout << "  \\$$$$$$  \\$$   \\$$  \\$$$$$$            \\$$      \\$$$$$$   \\$$$$$$ \n";
    cout << "\n\nFile to assemble >> ";

    string file_to_open;
    cin >> file_to_open;
    freopen((file_to_open).c_str(), "r", stdin);

    string inp;
    while (getline(cin, inp))
    {
        if (inp == "")
            continue;
        stringstream linestream(inp);
        string token;
        vector<string> tokens;
        while (getline(linestream, token, ' '))
        {
            if (token == "")
                continue;
            tokens.push_back((token.back() == ',') ? token.substr(0, token.length() - 1) : token);
        }
        tokenized_lines.push_back(tokens);
    }

    assemble();

    cout << "\n\nFILE:";
    int current_instruction = 0;
    for (int i = 0; i < program_counter; i++)
    {
        if (i == instruction_starts[current_instruction])
        {
            cout << "\n0x" << hex << i << "\t";
            current_instruction++;
        }
        if (i < instruction_stops[current_instruction - 1])
            cout << ((memory[i] <= 0xf) ? "0" : "") << hex << memory[i] << " ";
    }

    // display memory
    cout << "\n\nMEMORY:\n";
    for (int i = 0; i < program_counter; i++)
    {
        if (i % 16 == 0)
            cout << "0x" << hex << i << "\t";
        cout << ((memory[i] <= 0xf) ? "0" : "") << hex << memory[i] << (((i + 1) % 16) ? " " : "\n");
    }
    return 0;
}

/*
These are a set of maps to help go from the given assembly command to a respective binary mapping. They are
split by the type of output they would produce.
*/
unordered_map<string, int> registers;
unordered_map<string, int> arithmetic;
unordered_map<string, int> moves;
unordered_map<string, int> cmov;
unordered_map<string, int> j;
unordered_map<string, int> push_pop;
unordered_map<string, int> no_arg;


// TO DO:   ✅ finish directives
//          ✅ switch to little endian
//          ✅ ensure irmovl, rmmovl, mrmovl use 6 bytes
//          ✅ ensure jXX, call use 5 bytes
//          ✅ program counter
//          ❌ process symbolic names
//          ✅ process indirect addressing
//          ❌ bad input error handling
//          ✅ write to memory
//          ❌ better use of hashmaps
//          ❌ clean up for more concise

/*
Uses symbolic names:
✅  jump instructions (jXX, call)
✅  memory access instructions (rmmovl, mrmovl)
✅  immediate access (irmovl)
❌  assembler directives (.pos, .byte, .long, .word)
✅  overwriting old bytecode to work in symbolic names

Uses indirect addressing:
✅  memory access instructions (rmmovl, mrmovl). eg (rB) instead of 0(rB)
*/

vector<int> to_little_endian(int num)
{
    return {(num) & 0xff, (num >> 8) & 0xff, (num >> 16) & 0xff, (num >> 24) & 0xff};
}

vector<int> to_little_endian_long(long long num)
{
    return {(int)((num) & 0xff), (int)((num >> 8) & 0xff), (int)((num >> 16) & 0xff), (int)((num >> 24) & 0xff), (int)((num >> 32) & 0xff), (int)((num >> 40) & 0xff), (int)((num >> 48) & 0xff), (int)((num >> 56) & 0xff)};
}

void assemble()
{
    queue<pair<string, int>> symbolic_names_to_replace;
    for (auto line : tokenized_lines)
    {
        bool in_command = false;
        bool in_directive = false;

        string directive;
        string command;

        string commandType;

        int arg_no = 0;

        vector<int> to_add;
        int curr_byte = 0;

        for (auto token : line)
        {
            // if in directive, execute directive action (DONE)
            if (in_directive)
            {
                unsigned long long get_int;
                if (token.substr(0, 2) == "0x")
                    get_int = stoull(token, 0, 16);
                else
                    get_int = stoull(token);

                if (directive == ".byte")
                { // LITTLE ENDIAN (DONE)
                    vector<int> param = to_little_endian(get_int);
                    to_add.insert(to_add.end(), param.begin(), param.begin() + 1);
                }
                else if (directive == ".long" || directive == ".word")
                { // LITTLE ENDIAN (DONE)
                    vector<int> param = to_little_endian(get_int);
                    to_add.insert(to_add.end(), param.begin(), param.end());
                }
                else if (directive == ".quad")
                { // LITTLE ENDIAN (DONE)
                    vector<int> param = to_little_endian_long(get_int);
                    to_add.insert(to_add.end(), param.begin(), param.end());
                }
                else if (directive == ".align")
                {
                    program_counter = program_counter - (program_counter % get_int) + get_int;
                }
                else if (directive == ".pos")
                {
                    program_counter = get_int;
                }
                break;
            }
            // if in command, build command (NOT DONE)
            else if (in_command)
            {
                vector<int> param;
                stringstream linestream(token);
                string tmp;
                vector<string> splitaddr;
                if (commandType == "arithmetic" || commandType == "cmov")
                {
                    switch (arg_no++)
                    {
                    case 0:
                        to_add.push_back(registers[token] << 4);
                        break;
                    case 1:
                        to_add[1] += registers[token];
                    }
                }
                else if (commandType == "moves")
                {
                    if (command == "irmovl")
                    { // LITTLE ENDIAN
                        switch (arg_no++)
                        {
                        case 0:
                            to_add.push_back(0);
                            if (token[0] == '$')
                            { // immediate
                                token = token.substr(1, token.length());
                                param = to_little_endian(stoul(token));
                            }
                            else
                            { // symbolic
                                symbolic_names_to_replace.push(make_pair(token, program_counter + 2));
                                param = to_little_endian(0);
                            }
                            to_add.insert(to_add.end(), param.begin(), param.end());
                            break;
                        case 1:
                            to_add[1] = (0xf << 4) + registers[token];
                        }
                    }
                    else if (command == "rmmovl")
                    { // LITTLE ENDIAN
                        switch (arg_no++)
                        {
                        case 0:
                            to_add.push_back(registers[token] << 4);
                            break;
                        case 1:
                            if (token.find("(") != string::npos)
                            { // memory-addressed
                                while (getline(linestream, tmp, '('))
                                {
                                    if (tmp.back() == ')')
                                        tmp = tmp.substr(0, tmp.length() - 1);
                                    splitaddr.push_back(tmp);
                                }
                                to_add[1] += registers[splitaddr[1]];
                                param = to_little_endian(((splitaddr[0].size() != 0) ? stoul(splitaddr[0]) : 0));
                            }
                            else
                            { // symbolic
                                symbolic_names_to_replace.push(make_pair(token, program_counter + 2));
                                param = to_little_endian(0);
                            }
                            to_add.insert(to_add.end(), param.begin(), param.end());
                        }
                    }
                    else if (command == "mrmovl")
                    { // LITTLE ENDIAN
                        switch (arg_no++)
                        {
                        case 0:
                            if (token.find("(") != string::npos)
                            { // memory-addressed
                                while (getline(linestream, tmp, '('))
                                {
                                    if (tmp.back() == ')')
                                        tmp = tmp.substr(0, tmp.length() - 1);
                                    splitaddr.push_back(tmp);
                                }
                                to_add.push_back(registers[splitaddr[1]]);
                                param = to_little_endian(((splitaddr[0].size() != 0) ? stoul(splitaddr[0]) : 0));
                            }
                            else
                            { // symbolic
                                symbolic_names_to_replace.push(make_pair(token, program_counter + 2));
                                param = to_little_endian(0);
                            }
                            to_add.insert(to_add.end(), param.begin(), param.end());
                            break;
                        case 1:
                            to_add[1] += (registers[token] << 4);
                        }
                    }
                }
                else if (commandType == "j")
                { // LITTLE ENDIAN
                    symbolic_names_to_replace.push(make_pair(token, program_counter + 1));
                    param = to_little_endian(0);
                    to_add.insert(to_add.end(), param.begin(), param.end());
                }
                else if (commandType == "push_pop")
                {
                    to_add.push_back((registers[token] << 4) + 0xF);
                }
            }
            // if symbolic name, save current memory addr (program counter) (DONE)
            else if (token.back() == ':' && !isdigit(token.front()) && !(arithmetic.count(token) || cmov.count(token) || moves.count(token) || j.count(token) || push_pop.count(token) || no_arg.count(token)))
            {
                symbolic_names[token.substr(0, token.length() - 1)] = program_counter;
            }
            // else if in directive, do directive things (NOT DONE)
            else if (token.front() == '.')
            {
                in_directive = true;
                directive = token;
            }
            // else if command (NOT DONE)
            else if (arithmetic.count(token) || cmov.count(token) || moves.count(token) || j.count(token) || push_pop.count(token) || no_arg.count(token))
            {
                in_command = true;
                command = token;
                if (arithmetic.count(command))
                {
                    commandType = "arithmetic";
                    to_add.push_back(arithmetic[command]);
                }
                else if (cmov.count(command))
                {
                    commandType = "cmov";
                    to_add.push_back(cmov[command]);
                }
                else if (moves.count(command))
                {
                    commandType = "moves";
                    to_add.push_back(moves[command]);
                }
                else if (j.count(command))
                {
                    commandType = "j";
                    to_add.push_back(j[command]);
                }
                else if (push_pop.count(command))
                {
                    commandType = "push_pop";
                    to_add.push_back(push_pop[command]);
                }
                else if (no_arg.count(command))
                {
                    commandType = "no_arg";
                    to_add.push_back(no_arg[command]);
                }
            }
            // unrecognized instruction
            else
            {
                cout << "FATAL ERROR: token '" << token << "' not recognized";
                exit(0);
            }
        }

        // OUTPUT ASSEMBLED CODE (pre-symbolic names)
        /*
        cout << "0x" << hex << program_counter << "\t";
        ostringstream bytecode;
        for (auto x : to_add)
        {
            if (x <= 0xf)
                bytecode << "0";
            bytecode << hex << x << " ";
        }
        cout << left << setw(26) << setfill(' ') << bytecode.str();
        for (auto x : line)
            cout << x << " ";
        cout << "\n";
        */

        // WRITE TO MEMORY AND INCREMENT PC
        if (to_add.size() > 0)
        {
            instruction_starts.push_back(program_counter);
            instruction_stops.push_back(program_counter + to_add.size());
        }
        for (int i = 0; i < to_add.size(); i++)
            memory[program_counter + i] = to_add[i];
        program_counter += to_add.size();
    }

    while (!symbolic_names_to_replace.empty())
    {
        pair<string, int> current = symbolic_names_to_replace.front();
        symbolic_names_to_replace.pop();
        if (!symbolic_names.count(current.first))
        {
            cout << "FATAL ERROR, symbolic name '" << current.first << "' not recognized";
            exit(0);
        }
        int address = symbolic_names[current.first];
        vector<int> replacement = to_little_endian(address);
        for (int i = 0; i < 4; i++)
        {
            memory[current.second + i] = replacement[i];
        }
    }
}

void _init()
{
    symbolic_names["Stack"] = 0x100;

    registers["%eax"] = 0x0;
    registers["%ecx"] = 0x1;
    registers["%edx"] = 0x2;
    registers["%ebx"] = 0x3;
    registers["%esp"] = 0x4;
    registers["%ebp"] = 0x5;
    registers["%esi"] = 0x6;
    registers["%edi"] = 0x7;
    registers["%e8"] = 0x8;
    registers["%e9"] = 0x9;
    registers["%e10"] = 0x10;
    registers["%e11"] = 0x11;
    registers["%e12"] = 0x12;
    registers["%e13"] = 0x13;
    registers["%e14"] = 0x14;

    arithmetic["addl"] = 0x60;
    arithmetic["subl"] = 0x61;
    arithmetic["andl"] = 0x62;
    arithmetic["xorl"] = 0x63;

    cmov["rrmovl"] = 0x20;
    cmov["cmovle"] = 0x21;
    cmov["cmovl"] = 0x22;
    cmov["cmove"] = 0x23;
    cmov["cmovne"] = 0x24;
    cmov["cmovge"] = 0x25;
    cmov["cmovg"] = 0x26;

    moves["irmovl"] = 0x30;
    moves["rmmovl"] = 0x40;
    moves["mrmovl"] = 0x50;

    j["jmp"] = 0x70;
    j["jle"] = 0x71;
    j["jl"] = 0x72;
    j["je"] = 0x73;
    j["jne"] = 0x74;
    j["jge"] = 0x75;
    j["jg"] = 0x76;
    j["call"] = 0x80;

    push_pop["pushl"] = 0xA0;
    push_pop["popl"] = 0xB0;

    no_arg["halt"] = 0x00;
    no_arg["nop"] = 0x10;
    no_arg["ret"] = 0x90;
}