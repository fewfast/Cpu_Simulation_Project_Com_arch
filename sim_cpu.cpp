#include <iostream>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <algorithm>

using namespace std;

class CPU {
public:
    int registers[32] = {0};   // 32 Registers
    int PC = 0;               // Program Counter
    int memory[1024] = {0};    // Simple Memory 4KB
    unordered_map<string, function<void(int,int,int) >> instr_map;   // Map for Opcodes
    unordered_map<string, int> reg_map;  // Map for register -> index

    CPU() {
        registers[0] = 0; // Register 0 always 0
        initInstructionMap();
        initRegisterMap();

    }

    //  ค่า fix ไว้อยู่แล้ว
    void initRegisterMap() {
        reg_map["$zero"] = 0;
        reg_map[""] = 1;
        reg_map[""] = 2;
        reg_map[""] = 3;
        reg_map[""] = 4;
        reg_map[""] = 5;
        reg_map[""] = 6;
        reg_map[""] = 7;
        reg_map["$t0"] = 8;
        reg_map["$t1"] = 9; 
        reg_map["$t2"] = 10;
        reg_map["$t3"] = 11;
        reg_map[""] = 12;
        reg_map[""] = 13;
        reg_map[""] = 14;
        reg_map[""] = 15;
        reg_map["$s0"] = 16; 
        reg_map["$s1"] = 17; 
        reg_map["$s2"] = 18; 
        reg_map["$s3"] = 19;
        reg_map[""] = 20;
        reg_map[""] = 21;
        reg_map[""] = 22;
        reg_map[""] = 23;
        reg_map[""] = 24;
        reg_map[""] = 25;
        reg_map[""] = 26;
        reg_map[""] = 27;
        reg_map[""] = 28;
        reg_map[""] = 29;
        reg_map[""] = 30;
        reg_map[""] = 31;
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
            registers[rt] =memory[registers[rs]+offset / 4];
        };
        instr_map["sw"] = [this](int rt,int offset,int rs){
            memory[(registers[rs] + offset) / 4] = registers[rt];
        };
        instr_map["li"] = [this](int rt,int imm,int dummy){
            registers[rt] = imm;
        };
        instr_map["beq"] = [this](int rs, int rt, int offset) {
            if (registers[rs] == registers[rt]) {
                PC += offset * 4;  // คูณ 4 เพราะ MIPS ใช้ word-addressing
            }
        }; 
        instr_map["bne"] = [this](int rs, int rt, int offset) {
            if (registers[rs] != registers[rt]) {
                PC += offset * 4;  // คูณ 4 เพราะ MIPS ใช้ word-addressing
            }
        };
        instr_map["j"] = [this](int target, int dummy1, int dummy2) {
            PC = target * 4;    // คูณ 4 เพราะ MIPS ใช้ word-addressing
        };
        
        instr_map["jal"] = [this](int target, int dummy1, int dummy2) {
            registers[31] = PC + 4;  // บันทึกที่อยู่ถัดไปลงใน $ra
            PC = target * 4;
        };
        instr_map["jr"] = [this](int rs, int dummy1, int dummy2) {
            PC = registers[rs] ;  // ✅ กระโดดไปที่อยู่ที่เก็บใน rs
        };
        instr_map["and"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] & registers[rt];
        };
        instr_map["or"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] | registers[rt];
        };
        instr_map["slt"] = [this](int rd, int rs, int rt) {
            registers[rd] = (registers[rs] < registers[rt]) ? 1 : 0;
        };
    }

    void execute(string instruction);
    void printRegisters();
};
//function exucute
void CPU::execute(string instruction) {
    string op, rd, rs, rt;
    istringstream iss(instruction);
    
    // อ่านคำสั่งตัวแรกก,
    iss >> op;  
   

    //คำสั่งประเภท jump (J, Jal , Jr)
    if (op == "jr") {
        if (!(iss >> rs)) {
            throw runtime_error("Missing register in " + instruction);
        }
        if (reg_map.find(rs) == reg_map.end()) {
            throw runtime_error("Invalid register: " + rs);
        }
        int rs_index = reg_map[rs];
        instr_map["jr"](reg_map[rs], 0, 0);
        return;
    }
    
    if (op == "j" || op == "jal") {
        string target_str;
        if (!(iss >> target_str)) {
            throw runtime_error("Invalid jump format: " + instruction);
        }
        //ถ้าไม่เกี่ยวค่อยลบ
        int target = stoi(target_str);
        instr_map[op](target, 0, 0);
        return;
    }

    //คำสั่งประเภท branch (BEQ, BNE)  
    if (op == "beq" || op == "bne") {  
        string rs_str, rt_str, offset_str;
        if (!(iss >> rs_str >> rt_str >> offset_str)) {
            throw runtime_error("Invalid " + op + " format: " + op + " $rs, $rt, offset");
        }//ถ้า check ข้างบนแล้วค่ามันผิดลบ + op + " $rs, $rt, offset"
        if (reg_map.find(rs_str) == reg_map.end() || reg_map.find(rt_str) == reg_map.end()) {
            throw runtime_error("Invalid register in " + op + ": " + rs_str + ", " + rt_str);
        }//ถ้า check ข้างบนแล้วค่ามันผิดลบ  ": " + rs_str + ", " + rt_str
        int rs_index = reg_map[rs_str];
        int rt_index = reg_map[rt_str];
        int offset = stoi(offset_str);
        instr_map[op](rs_index, rt_index, offset);
        return;
    }

    //คำสั่ง Load Immediate (LI)
    if(op == "li"){
        string rt_str ,imm_str;
        if(!(iss >> rt_str >> imm_str)){
            cout << op + rt_str + imm_str ;
            throw runtime_error("Invalid LI");
        }
        if(reg_map.find(rt_str) == reg_map.end()){
            throw runtime_error("Runtime:" +rt_str);
        }
        int rt_index = reg_map[rt_str];
        int imm = stoi(imm_str);
        
        instr_map["li"](rt_index,imm,0); 
        return;
    }

    //คำสั่ง Load/Store (LW , Sw)
    if (op == "lw" || op == "sw") {
        string rt_str, offset_rs;
        if (!(iss >> rt_str)) throw runtime_error("Missing destination register in " + instruction);
        getline(iss >> ws, offset_rs);
        size_t open_paren = offset_rs.find('('), close_paren = offset_rs.find(')');
        if (open_paren == string::npos || close_paren == string::npos)
            throw runtime_error("Invalid format for " + op);
        string offset_str = offset_rs.substr(0, open_paren);
        string rs_str = offset_rs.substr(open_paren + 1, close_paren - open_paren - 1);
        if (reg_map.find(rt_str) == reg_map.end() || reg_map.find(rs_str) == reg_map.end())
            throw runtime_error("Invalid register");
        instr_map[op](reg_map[rt_str], stoi(offset_str), reg_map[rs_str]);
        return;
    }

    if (!(iss >> rd >> rs >> rt)) throw runtime_error("Invalid format for " + op);
    if (reg_map.find(rd) == reg_map.end() || reg_map.find(rs) == reg_map.end() || reg_map.find(rt) == reg_map.end())
        throw runtime_error("Invalid register in " + op);
            
        // เรียกใช้งาน lambda function ตามคำสั่ง
        instr_map[op](reg_map[rd], reg_map[rs], reg_map[rt]);
    }


void CPU::printRegisters() {
    cout << "Register values:\n";

    vector<pair<string, int>> sorted_registers;
    for (const auto &r : reg_map) {
        sorted_registers.push_back({r.first, r.second});
    }

    // เรียงลำดับตามค่าของรีจิสเตอร์ (index)
    sort(sorted_registers.begin(), sorted_registers.end(), 
         [](const pair<string, int> &a, const pair<string, int> &b) {
             return a.second < b.second;
         });

    // แสดงผลตามลำดับที่ถูกต้อง
    for (const auto &r : sorted_registers) {
        cout << r.first << " = " << registers[r.second] << "\n";
    }
}

int main() {
    CPU cpu;
    string command;

    cout << "Enter assembly instuction (type 'exit' to quit):\n";
  
    while (true) {
        cout << "> ";
        getline(cin, command);
        if (command == "exit") break;
        try {
            cpu.execute(command);
            cpu.printRegisters();
        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
        }
    }

    
    return 0;
}