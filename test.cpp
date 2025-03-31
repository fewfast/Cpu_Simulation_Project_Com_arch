#include <iostream>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <vector>
#include <algorithm>

using namespace std;

class CPU {
public:
    int registers[32] = {0};     // 32 Registers
    int PC = 0;                  // Program Counter, ใช้เป็น index ใน vector<string> ของโปรแกรม
    int memory[1024] = {0};      // Simple Memory 4KB
    unordered_map<string, function<void(int, int, int)>> instr_map; // Map สำหรับฟังก์ชันคำสั่ง
    unordered_map<string, int> reg_map;  // Map สำหรับชื่อ register -> index
    vector<string> program;      // เก็บโปรแกรม assembly ที่รับเข้ามา

    CPU() {
        registers[0] = 0; // $zero ต้องมีค่า 0 เสมอ
        initRegisterMap();
        initInstructionMap();
    }

    void initRegisterMap() {
        // กำหนดแมปของ register ตาม MIPS convention
        reg_map["$zero"] = 0;
        reg_map["$at"] = 1;
        reg_map["$v0"] = 2; reg_map["$v1"] = 3;
        reg_map["$a0"] = 4; reg_map["$a1"] = 5; reg_map["$a2"] = 6; reg_map["$a3"] = 7;
        reg_map["$t0"] = 8; reg_map["$t1"] = 9; reg_map["$t2"] = 10; reg_map["$t3"] = 11;
        reg_map["$t4"] = 12; reg_map["$t5"] = 13; reg_map["$t6"] = 14; reg_map["$t7"] = 15;
        reg_map["$s0"] = 16; reg_map["$s1"] = 17; reg_map["$s2"] = 18; reg_map["$s3"] = 19;
        reg_map["$s4"] = 20; reg_map["$s5"] = 21; reg_map["$s6"] = 22; reg_map["$s7"] = 23;
        reg_map["$t8"] = 24; reg_map["$t9"] = 25;
        reg_map["$k0"] = 26; reg_map["$k1"] = 27;
        reg_map["$gp"] = 28;
        reg_map["$sp"] = 29;
        reg_map["$fp"] = 30;
        reg_map["$ra"] = 31;
    }

    void initInstructionMap(){
        instr_map["add"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] + registers[rt];
            PC++;  // ไปยังคำสั่งถัดไป
        };
        instr_map["sub"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] - registers[rt];
            PC++;
        };
        instr_map["mul"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] * registers[rt];
            PC++;
        };
        instr_map["lw"] = [this](int rt, int offset, int rs) {
            // สมมุติ offset ที่ได้รับเป็น byte offset; แต่เราจะแปลงเป็น index ด้วยหาร 4
            registers[rt] = memory[(registers[rs] + offset) / 4];
            PC++;
        };
        instr_map["sw"] = [this](int rt, int offset, int rs) {
            memory[(registers[rs] + offset) / 4] = registers[rt];
            PC++;
        };
        instr_map["li"] = [this](int rt, int imm, int) {
            registers[rt] = imm;
            PC++;
        };
        // Branch if equal: ถ้า register[rs] == register[rt] ให้กระโดด (PC = PC + 1 + offset), ไม่เช่นนั้น PC++ 
        instr_map["beq"] = [this](int rs, int rt, int offset) {
            if (registers[rs] == registers[rt])
                PC = PC + 1 + offset;
            else
                PC++;
        };
        // Branch if not equal
        instr_map["bne"] = [this](int rs, int rt, int offset) {
            if (registers[rs] != registers[rt])
                PC = PC + 1 + offset;
            else
                PC++;
        };
        // Jump: เปลี่ยน PC ให้เท่ากับ target (target นี้เป็น index ใน vector)
        instr_map["j"] = [this](int target, int, int) {
            PC = target;
        };
        // Jump and Link: บันทึก PC+1 ลงใน $ra แล้วกระโดดไป target
        instr_map["jal"] = [this](int target, int, int) {
            registers[31] = PC + 1;
            PC = target;
        };
        // Jump Register: กระโดดไปที่อยู่ที่เก็บในรีจิสเตอร์ที่ระบุ
        instr_map["jr"] = [this](int rs, int, int) {
            PC = registers[rs];
        };
        instr_map["and"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] & registers[rt];
            PC++;
        };
        instr_map["or"] = [this](int rd, int rs, int rt) {
            registers[rd] = registers[rs] | registers[rt];
            PC++;
        };
        instr_map["slt"] = [this](int rd, int rs, int rt) {
            registers[rd] = (registers[rs] < registers[rt]) ? 1 : 0;
            PC++;
        };
    }

    // รับคำสั่ง (instruction) ที่เป็น string แล้วประมวลผล
    void execute(string instruction) {
        string op, operand1, operand2, operand3;
        istringstream iss(instruction);
        iss >> op;

        // กรณีคำสั่ง jump (jr, j, jal)
        if (op == "jr") {
            if (!(iss >> operand1))
                throw runtime_error("Missing register in " + instruction);
            if (reg_map.find(operand1) == reg_map.end())
                throw runtime_error("Invalid register: " + operand1);
            instr_map["jr"](reg_map[operand1], 0, 0);
            return;
        }
        if (op == "j" || op == "jal") {
            string target_str;
            if (!(iss >> target_str))
                throw runtime_error("Invalid jump format: " + instruction);
            int target = stoi(target_str);
            instr_map[op](target, 0, 0);
            return;
        }

        // กรณี branch (beq, bne)
        if (op == "beq" || op == "bne") {
            string rs_str, rt_str, offset_str;
            if (!(iss >> rs_str >> rt_str >> offset_str))
                throw runtime_error("Invalid " + op + " format: " + op + " $rs, $rt, offset");
            if (reg_map.find(rs_str) == reg_map.end() || reg_map.find(rt_str) == reg_map.end())
                throw runtime_error("Invalid register in " + op + ": " + rs_str + ", " + rt_str);
            int rs_index = reg_map[rs_str];
            int rt_index = reg_map[rt_str];
            int offset = stoi(offset_str);
            instr_map[op](rs_index, rt_index, offset);
            return;
        }

        // กรณีคำสั่ง Load Immediate (li)
        if (op == "li") {
            string rt_str, imm_str;
            if (!(iss >> rt_str >> imm_str))
                throw runtime_error("Invalid LI format: " + instruction);
            if (reg_map.find(rt_str) == reg_map.end())
                throw runtime_error("Invalid register in LI: " + rt_str);
            int rt_index = reg_map[rt_str];
            int imm = stoi(imm_str);
            instr_map["li"](rt_index, imm, 0);
            return;
        }

        // กรณีคำสั่ง Load/Store (lw, sw) รูปแบบ: lw $rt, offset($rs)
        if (op == "lw" || op == "sw") {
            string rt_str, offset_rs;
            if (!(iss >> rt_str))
                throw runtime_error("Missing destination register in " + instruction);
            getline(iss >> ws, offset_rs);
            size_t open_paren = offset_rs.find('('), close_paren = offset_rs.find(')');
            if (open_paren == string::npos || close_paren == string::npos)
                throw runtime_error("Invalid format for " + op);
            string offset_str = offset_rs.substr(0, open_paren);
            string rs_str = offset_rs.substr(open_paren + 1, close_paren - open_paren - 1);
            if (reg_map.find(rt_str) == reg_map.end() || reg_map.find(rs_str) == reg_map.end())
                throw runtime_error("Invalid register in " + op);
            instr_map[op](reg_map[rt_str], stoi(offset_str), reg_map[rs_str]);
            return;
        }

        // กรณีคำสั่งประเภทที่มี 3 operands: add, sub, mul, and, or, slt เป็นต้น
        if (!(iss >> operand1 >> operand2 >> operand3))
            throw runtime_error("Invalid format for " + op);
        if (reg_map.find(operand1) == reg_map.end() || reg_map.find(operand2) == reg_map.end() || reg_map.find(operand3) == reg_map.end())
            throw runtime_error("Invalid register in " + op);
        instr_map[op](reg_map[operand1], reg_map[operand2], reg_map[operand3]);
    }

    // แสดงค่าของรีจิสเตอร์โดยเรียงตามลำดับ index
    void printRegisters() {
        cout << "Register values:\n";
        vector<pair<string, int>> sorted_registers;
        for (const auto &r : reg_map) {
            sorted_registers.push_back({r.first, r.second});
        }
        sort(sorted_registers.begin(), sorted_registers.end(),
            [](const pair<string, int> &a, const pair<string, int> &b) {
                return a.second < b.second;
            });
        for (const auto &r : sorted_registers) {
            cout << r.first << " = " << registers[r.second] << "\n";
        }
    }

    // รันโปรแกรมตามที่เก็บไว้ใน vector<string> program โดยใช้ PC เป็นตัวควบคุม
    void run() {
        while (PC < program.size()) {
            string instr = program[PC];
            cout << "Executing: " << instr << "\n";
            execute(instr);
            printRegisters();
            cout << "PC = " << PC << "\n";
            cout << "--------------------------\n";
        }
    }

    // โหลดโปรแกรม assembly จาก vector<string>
    void loadProgram(const vector<string> &prog) {
        program = prog;
    }
};

int main() {
    CPU cpu;
    vector<string> program = {
        "li $t0 5",
        "li $t1 5",
        "beq $t0 $t1 2",   // ถ้า $t0 == $t1, กระโดดไปข้าม 2 คำสั่ง
        "li $t3 99",        // คำสั่งนี้จะถูกข้ามถ้า branch ทำงาน
        "li $s1 99"         // คำสั่งนี้จะถูกข้ามถ้า branch ทำงาน
    };

    cpu.loadProgram(program);
    cpu.run();

    return 0;
}
