#include <iostream>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <algorithm>
#include <vector>

using namespace std;

class CPU {
public:
    int registers[32] = {0};   //  สร้าง 32 Registers และตั้งให้ทุกช่องใน Array มีค่าเป็น 0
    int PC = 0;               //  Program Counter บอกตำแหน่งปัจจุบันของ Command ที่ Process อยู่
    int memory[1024] = {0};    //  สร้าง Ram ขนาด 4KB โดยที่ 1 ช่อง array = 4 byte , 4*1024 = 4096B , = 4KB
    unordered_map<string, function<void(int,int,int) >> instr_map;   /*  สร้างตัวแปรชื่อ instr_map มาเก็บ command list
                                                                        unordered_map = การสร้าง map ที่ไม่ได้เรียงตามตัวอักษรถ้าเรียงตามตัวอักษรอัตโนมัติจะใช้ map
                                                                        unordered_map<string, function<void(int, int, int)>> เป็น hash map
                                                                        hash map = โครงสร้างข้อมูลที่มีรูปแบบเป็น คู่ key กับ value เช่น map["Alice"] = 30;
                                                                        function<void(int, int, int)> = เก็บ ฟังก์ชันที่ไม่คืนค่า (void) และรับ พารามิเตอร์ 3 ตัว ซึ่งแต่ละตัวมีประเภทเป็น int
                                                                    */
    unordered_map<string, int> reg_map;  // สร้าง map ที่ไม่เรียงตามอักษร ไว้เก็บค่า value แต่ละ register

    CPU() {
        registers[0] = 0; // Register 0 always 0
        initInstructionMap(); // ประกาศ function ทิ้งไว้
        initRegisterMap(); // ประกาศ function ทิ้งไว้

    }

    //  ค่า fix ไว้อยู่แล้ว
    //  ค่า fix ไว้อยู่แล้ว(initRegisterMap ห้ามแก้!)
    void initRegisterMap() // ทำให้ function initRegisterMap() กำหนดข้อมูลใน map register
    {

        reg_map["$zero"] = 0; //$Zero Register เป็น Constant มีค่า 0 (เปลี่ยนแปลงไม่ได้)
        reg_map["$at"] = 1;   // "$Assembler Temporary ทำหน้าที่เก็บข้อมูลชั่วคราวระหว่างการแปลรหัส assembly หรือช่วยในการคำนวณที่ซับซ้อนระหว่างการแปลรหัส

        // $Value เก็บค่า Result จากฟังก์ชันหรือการคำนวณ ใช้ใน system calls เพื่อบอก OS ว่าต้องการให้ทำอะไร ($v0 เก็บผลลัพธ์หลัก)($v1 เก็บผลลัพธ์รอง)
        reg_map["$v0"] = 2;
        reg_map["$v1"] = 3;
        // $Arguments ข้อมูลที่ส่งให้กับฟังก์ชันเพื่อให้ฟังก์ชันนั้นสามารถทำงานได้
        reg_map["$a0"] = 4;
        reg_map["$a1"] = 5;
        reg_map["$a2"] = 6;
        reg_map["$a3"] = 7;
        // $Temporary registers ชั่วคราว ที่ใช้ในการเก็บข้อมูลระหว่างการคำนวณหรือลำดับการดำเนินงานต่าง ๆ

        reg_map["$t0"] = 8;
        reg_map["$t1"] = 9;
        reg_map["$t2"] = 10;
        reg_map["$t3"] = 11;

        reg_map["$t4"] = 12;
        reg_map["$t5"] = 13;
        reg_map["$t6"] = 14;
        reg_map["$t7"] = 15;
        // $Saved Registers ที่เก็บค่าไว้ระหว่างการเรียกฟังก์ชัน
        reg_map["$s0"] = 16;
        reg_map["$s1"] = 17;
        reg_map["$s2"] = 18;
        reg_map["$s3"] = 19;
        reg_map["$s4"] = 20;
        reg_map["$s5"] = 21;
        reg_map["$s6"] = 22;
        reg_map["$s7"] = 23;

        // $Temporary registers ชั่วคราว ที่ใช้ในการเก็บข้อมูลระหว่างการคำนวณหรือลำดับการดำเนินงานต่าง ๆ
        reg_map["$t8"] = 24;
        reg_map["$t9"] = 25;

        // Special Registers ตัวแปร พิเศษ
        //$Kernel registers ใช้โดย OS ถูกสงวนไว้สำหรับการใช้งานของระบบปฏิบัติการ
        reg_map["$k0"] = 26;
        reg_map["$k1"] = 27;

        reg_map["$gp"] = 28; // Global Pointer ชี้ไปยังตำแหน่งเริ่มต้นของ global data segment ซึ่งเป็นส่วนของหน่วยความจำที่เก็บข้อมูลที่สามารถเข้าถึงได้จากทั่วทั้งโปรแกรม
        reg_map["$sp"] = 29; // Stack Pointer ชี้ไปยังตำแหน่งปัจจุบันของ stack ข้อมูลจะถูก push (บันทึก) หรือ pop (ดึงออก) ตามลำดับของ stack (LIFO
        reg_map["$fp"] = 30; // Frame Pointer ชี้ไปยังตำแหน่งเริ่มต้นของ stack frame ของฟังก์ชันที่กำลังทำงาน
        reg_map["$ra"] = 31; // Return Address ใช้เก็บ ที่อยู่ของคำสั่งถัดไป หลังจากที่เราเรียกฟังก์ชัน (หรือคำสั่ง) ใหม่
    }

    //[this] การ "จับ" (capture) ตัวแปรของคลาสหรืออ็อบเจกต์ที่เรียกใช้งาน lambda function ในขณะนั้น
    void initInstructionMap() // ที่เก็บการจับคู่ระหว่าง ชื่อคำสั่ง (instruction) กับ ฟังก์ชันที่ใช้จัดการคำสั่ง (lambda functions)
    {
        // บวก add $t0, $t1, $t2 [$t0 = $t1 + $t2]
        instr_map["add"] = [this](int rd, int rs, int rt)
        {
            registers[rd] = registers[rs] + registers[rt];
        };

        // ลบ sub $t0, $t1, $t2 [$t0 = $t1 - $t2]
        instr_map["sub"] = [this](int rd, int rs, int rt)
        {
            registers[rd] = registers[rs] - registers[rt];
        };

        // คูณ mul $t0, $t1, $t2 [$t0 = $t1 * $t2]
        instr_map["mul"] = [this](int rd, int rs, int rt)
        {
            registers[rd] = registers[rs] * registers[rt];
        };

        /*  Load Word โหลดข้อมูลจาก Memory registers
            lw $t0, offset($t1) [$t0 = memory[$t1 + offset]]
            $t0: รีจิสเตอร์ที่ใช้เก็บข้อมูล
            memory[$t1 + offset]:  ที่อยู่ในหน่วยความจำที่ต้องการโหลดข้อมูลมา ($t1เป็นเลข Word offset เป็นเลข byte)
            offset มีการเปลี่ยนจาก byte เป็น word โดยการหาร 4
        */
        instr_map["lw"] = [this](int rt, int offset, int rs)
        {
            registers[rt] = memory[(registers[rs] + offset) / 4];
        };

        /*  Store Word ใช้ในการเก็บค่าจาก register ลง Memory
            sw $t0, offset($s1) [[$s1 + offset] = $t0]
            เก็บค่าใน $t0 ลง memory[$s1 + offset]
            memory[$s1 + offset]:  ที่อยู่ในหน่วยความจำที่ต้องการ save ($s1เป็นเลข Word offset เป็นเลข byte)
            offset มีการเปลี่ยนจาก byte เป็น word โดยการหาร 4
        */
        instr_map["sw"] = [this](int rt, int offset, int rs)
        {
            memory[(registers[rs] + offset) / 4] = registers[rt];
        };

        /*  Load Immediate ใส่ค่าคงที่ลงใน register
            li $t0, imm [$t0 = imm]
        */
        instr_map["li"] = [this](int rt, int imm, int dummy)
        {
            registers[rt] = imm;
        };
      
        /*  Branch if Equal กระโดดไปยังตำแหน่งอื่นของโปรแกรม หากค่าของรีจิสเตอร์สองตัวเท่ากัน
            beq $rs, $rt, offset [if register[rs] == register[rt] กระโดดไปที่ PC + (offset*4)]
            PC ใช้เป็น Byte Address offset เลย *4
        */
        instr_map["beq"] = [this](int rs, int rt, int offset)
        {
            if (registers[rs] == registers[rt])
            {
                PC += offset * 4;
            }
        };

        /*  Branch if Not Equal กระโดดไปยังตำแหน่งอื่นของโปรแกรม หากค่าของรีจิสเตอร์สองตัวไม่เท่ากัน
            bne $rs, $rt, offset [if register[rs] != register[rt] กระโดดไปที่ PC + (offset*4)]
            PC ใช้เป็น Byte Address offset เลย *4
        */
        instr_map["bne"] = [this](int rs, int rt, int offset)
        {
            if (registers[rs] != registers[rt])
            {
                PC += offset * 4;
            }
        };

        //  j target | Jump กระโดดไปยัง target ,PC ใช้ Byte address , target เป็น word address เลยต้อง *4
        instr_map["j"] = [this](int target, int dummy1, int dummy2)
        {
            PC = target * 4;
        };

        //  jal target |Jump and Link กระโดดไปยัง target จากนั้น บันทึกค่าที่อยู่ถัดไป(PC + 4) ลง $ra,PC ใช้ Byte address , target เป็น word address เลยต้อง *4
        instr_map["jal"] = [this](int target, int dummy1, int dummy2)
        {
            registers[31] = PC + 4;
            PC = target * 4;
        };

        //  jr register |Jump Register กระโดดไปยังที่อยู่ในรีจิสเตอร์ที่ระบุ
        instr_map["jr"] = [this](int rs, int dummy1, int dummy2)
        {
            PC = registers[rs]; // ✅ กระโดดไปที่อยู่ที่เก็บใน rs
        };

        /*  And นำค่า 2 ค่าใน register มา and กันและเก็บผลลัพธ์ในรีจิสเตอร์ เป็นการตรวจสอบค่าบิตแต่ละบิตในสองตัวเลข ถ้าบิตทั้งสองเป็น 1 ผลลัพธ์จะเป็น 1 แต่ถ้าไม่ใช่ ผลลัพธ์จะเป็น 0
            and $rd, $rs, $rt [$rd = $rs and $rt]
            ผลลัพธ์ default จะมีค่าเป็นเลขฐาน 2
        */
        instr_map["and"] = [this](int rd, int rs, int rt)
        {
            registers[rd] = registers[rs] & registers[rt];
        };

        /*  Or นำค่า 2 ค่าใน register มา or กันและเก็บผลลัพธ์ในรีจิสเตอร์ เป็นการตรวจสอบค่าบิตแต่ละบิตในสองตัวเลข ถ้าบิตอันใดอันหนึ่งเป็น 1 ผลลัพธ์จะเป็น 1 แต่ถ้าไม่ใช่ ผลลัพธ์จะเป็น 0
            or $t0, $t1, $t2 [ $t0 = $t1 or $t2]
            ผลลัพธ์ default จะมีค่าเป็นเลขฐาน 2
        */
        instr_map["or"] = [this](int rd, int rs, int rt)
        {
            registers[rd] = registers[rs] | registers[rt];
        };

        /*  Set on Less Than กำหนดค่าเป็น 1 ถ้าค่าแรกน้อยกว่าค่า 2
            slt $t0, $t1, $t2 [ $t0 = $t1 compare $t2]
        */
        instr_map["slt"] = [this](int rd, int rs, int rt)
        {
            registers[rd] = (registers[rs] < registers[rt]) ? 1 : 0;
        };
    }
    void execute(string instruction); // สร้าง function เปล่าๆ มาเขียนแยกทีหลัง ทำงานโดยใช้ string
    void printRegisters();            // สร้าง function เปล่าๆ มาเขียนแยกทีหลัง เป็น function void
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
    
    vector<pair<int, string>> sorted_registers;// เก็บค่าตาม index
    for (const auto &r : reg_map) {
        sorted_registers.push_back({r.second, r.first});
    }

    // เรียงลำดับตามค่า index ของ reg_map (int) จาก$zero ไป $ra
    sort(sorted_registers.begin(), sorted_registers.end());

    // แสดงผลตามลำดับ index ที่ถูกต้อง
    for (const auto &r : sorted_registers) {
        cout << r.second << " = " << registers[r.first] << "\n";
    }
    cout << "PC = " << PC << endl;
}

int main() // start
{
    CPU cpu;
    string command;

    while (true)
    {
        // program guide
        cout << "-----Instruction List-----\n";
        cout << "add, sub, lw, sw, beq, bne, j, jal, jr, and, or, slt, li\n";
        cout << "Enter assembly instuction (type 'exit' to quit):\n";
        cout << "> ";
        // read string
        getline(cin, command);
        if (command == "exit") // if input string(command)=exit break while
        {
            break;
        }
        else
        {
            try
            {
                cpu.execute(command); // ส่ง command ไป function execute เพื่อเลือกใช้คำสั่ง
                cpu.printRegisters();
            }
            catch (const exception &e) // errorhandle & report
            {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }
    return 0;
}