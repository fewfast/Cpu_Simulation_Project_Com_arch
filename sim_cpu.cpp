#include <iostream>
#include <unordered_map>
#include <sstream>
#include <functional>

using namespace std;

class CPU {
public:
    int registers[32] = {0};   // 32 Registers
    int PC = 0;               // Program Counter
    int memory[1024] = {0};    // Simple Memory 4KB
    unordered_map<string, function<void(int,int,int)>> instr_map;   // Map for Opcodes
    unordered_map<string, int> reg_map;  // Map for register -> index

    CPU() {
        registers[0] = 0; // Register 0 always 0
        initInstructionMap();
        initRegisterMap();

    }


    void initRegisterMap() {
        reg_map["$zero"] = 0;
        reg_map["$t0"] = 8;
        reg_map["$t1"] = 9; 
        reg_map["$t2"] = 10;
        reg_map["$t3"] = 11;
        reg_map["$s0"] = 16; 
        reg_map["$s1"] = 17; 
        reg_map["$s2"] = 18; 
        reg_map["$s3"] = 19;
    }

    void initInstructionMap(){
        instr_map["add"] = [this](int rd, int rs,int rt){
            registers[rd] = registers[rs] + registers[rt];
        };
        instr_map["sub"] = [this](int rd,int rs, int rt){
            registers[rd] = registers[rs] - registers[rt];
        };
        instr_map["mul"] = [this](int rd,int  rs,int rt){
            registers[rd] = registers[rs] * registers[rt];
        };
        instr_map["lw"] = [this](int rt,int offset,int rs){
            registers[rt] =memory[registers[rs]+offset];
        };
        instr_map["li"] = [this](int rt,int imm,int dummy){
            registers[rt] = imm;
        };

    }

    void execute(string instruction);
};

void CPU::execute(string instruction) {
    string op, rd, rs, rt;
    istringstream iss(instruction);
    // แยกคำสั่งออกเป็นส่วนๆ&ลบ,
    iss >> op;  

    if(op == "li"){
        string rt_str ,imm_str;
        getline(iss >> ws,rt_str,',');
        getline(iss >> ws,imm_str);
        if(reg_map.find(rt_str)== reg_map.end()){
            throw runtime_error("Invalid destination: "+rt_str);
        }
        int rt_index =reg_map[rt_str];
        int imm;
        try{
            imm =stoi(imm_str);
        }catch(...){
            throw runtime_error("Incalid immediate value:"+imm_str);
        }
        instr_map["li"](rt_index,imm,0);
        return;
    }

    if(op == "lw " || op == "sw"){
        string rd_str,offset_rs;
        if (!(iss >> rd_str)){
            throw runtime_error("Missing destination register in " + instruction);
        }
        getline(iss >> ws,offset_rs);
        size_t open_paren = offset_rs.find('(');
        size_t close_paren = offset_rs.find(')');
        if (open_paren == string::npos || close_paren == string::npos){
            throw runtime_error("Invalid format for " + op + " instruction. Expected: "+op+"$rd,offset($rs)");
        }
        string offset_str = offset_rs.substr(0,open_paren);
        string rs_str = offset_rs.substr(open_paren + 1,close_paren - open_paren -1);
        if (reg_map.find(rd_str) == reg_map.end()){
            throw runtime_error("Invalid destination reguster: "+ rd_str);
        }
        if (reg_map.find(rs_str) == reg_map.end()){
            throw runtime_error("Incalid source register: "+rs_str);            
        }
        int offset;
        try{
            offset = stoi(offset_str);
        }catch(...){
            throw runtime_error("Incalid offset valie: " + offset_str);
        }
        int rd_index = reg_map[rd_str];
        int rs_index =reg_map[rs_str];

        if(op == "lw"){
            registers[rd_index] = memory[offset + registers[rs_index]];
        }

        if (op == "sw"){
            memory[offset + registers[rs_index]] = registers[rd_index];
        }else{
            cout << "error lw&sw";
        }

        return;
    }else{

        if (!getline(iss >> ws, rd, ',')) {
            throw runtime_error("Missing first operand in " + instruction);
        }
        if (!getline(iss >> ws, rs, ',')) {
            throw runtime_error("Missing second operand in " + instruction);
        }
        if (!getline(iss >> ws, rt)) {
            throw runtime_error("Missing third operand in " + instruction);
        }

        if (reg_map.find(rd) == reg_map.end()) {
            throw runtime_error("Invalid destination register: " + rd);
        }
        if (reg_map.find(rs) == reg_map.end()) {
            throw runtime_error("Invalid first source register: " + rs);
        }
        if (reg_map.find(rt) == reg_map.end()) {
            throw runtime_error("Invalid second source register: " + rt);
        }
            
        int rd_index = reg_map[rd];
        int rs_index = reg_map[rs];
        int rt_index = reg_map[rt];

        if (instr_map.find(op) == instr_map.end()) {
            throw runtime_error("Instruction not supported: " + op);
        }
            
        // เรียกใช้งาน lambda function ตามคำสั่ง
        instr_map[op](rd_index, rs_index, rt_index);
    }
}

int main() {
    CPU cpu;

    // กำหนดค่าเริ่มต้นให้กับบาง register
    cpu.registers[9] = 5;  // $t1 = 5
    cpu.registers[10] = 3; // $t2 = 3
    cpu.registers[18] = 10; // $s2 = 10
    cpu.registers[19] = 7;  // $s3 = 7



    cpu.execute("add $t0, $t1, $t2");  // $t0 = $t1 + $t2 = 5 + 3 = 8
    cpu.execute("sub $s1, $s2, $s3");  // $s1 = $s2 - $s3 = 10 - 7 = 3



    cout << "t0: " << cpu.registers[8] << endl;  // = 8
    cout << "s1: " << cpu.registers[17] << endl; // = 3


    cout << "t3: " << cpu.registers[11] << endl; 
    cpu.execute("li $t3, 99");
    cout << "t3: " << cpu.registers[11] << endl;  // = 99
    
    return 0;
}