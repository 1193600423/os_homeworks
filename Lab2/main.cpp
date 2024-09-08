#include <iostream>
#include <string>
#include<vector>
#include <sstream>
#include <stack>
#include <cstring>

using namespace std;
const char* imgPath = "lab2.img";
unsigned BytsPerSec; // 每扇区字节数
unsigned SecPerClus; // 每簇扇区数
unsigned RsvdSecCnt; // Boot记录占用的扇区数
unsigned FATSz16; // 每个FAT表所占扇区数
unsigned NumFATs; // FAT表个数
unsigned RootEntCnt; // 根目录最大文件数
unsigned BytsPerClus; // 每簇字节数
//区域启示
unsigned FATStart;
unsigned ROOTStart;
unsigned DATAStart;

struct FileTreeNode
{
    FileTreeNode* neighbor = NULL;
    FileTreeNode* child = NULL;
    string fileName = "";
    bool isDir= true;
    unsigned fstClus = 0;
    unsigned childDir = 0;
    unsigned childFile = 0;
    unsigned fileSize = 0;
};
struct RootEntry {
    char DIR_Name[11];
    // char DIR_Name[8];
    // char DIR_Name_Ext[3];
    char DIR_Attr[1];
    char preserved[10];
    char DIR_WrtTime[2];
    char DIR_WrtDate[2];
    char DIR_FstClus[2];
    char DIR_FileSize[4];
};
struct BPB {
    char BPB_BytsPerSec[2];    // 每扇区字节数
    char BPB_SecPerClus[1];     // 每簇扇区数
    char BPB_RsvdSecCnt[2];    // Boot记录占用的扇区数
    char BPB_NumFATs[1];        // FAT表个数
    char BPB_RootEntCnt[2];    // 根目录最大文件数
    char BPB_TotSec16[2];      // 扇区总数
    char BPB_Media[1];          // 介质描述符
    char BPB_FATSz16[2];       // 每个FAT表所占扇区数
    char BPB_SecPerTrk[2];     // 每磁道扇区数（Sector/track）
    char BPB_NumHeads[2];      // 磁头数（面数）
    char BPB_HiddSec[4];       // 隐藏扇区数
    char BPB_TotSec32[4];      // 如果BPB_ToSec16为0，该值为扇区数
};


extern "C" void my_print (const char*, const int);
// 函数声明
void print_out(string str);
int char2int ( char* source, int begin, int size);

bool BPBLoad(FILE* filestream, BPB* bpb);
bool FileTreeLoad(FILE* filestream, FileTreeNode* fileTree);
bool inputHandle(string input, string* command, string* option, string* parameter);
bool FileCd(FileTreeNode* fileNode, string* parameter, stack<FileTreeNode>& fileStack);
bool CatHandle(FileTreeNode* fileNode, FILE* filestream);
bool LsHandle(FileTreeNode* fileNode, bool option, stack<FileTreeNode>& fileStack);
bool pathPrint(stack<FileTreeNode> fileStack);

int main() {
    FILE* filestream = fopen(imgPath,"rb");
    //BPB
    fseek(filestream, 11, SEEK_SET);
    BPB* bpb = new BPB;
    BPBLoad(filestream, bpb);
    
    // 文件树加载
    fseek(filestream,ROOTStart,SEEK_SET);
    FileTreeNode* fileTree = new FileTreeNode;
    FileTreeLoad(filestream, fileTree);
    
    // 输入，响应
    while (true)
    {
        print_out(">");
        string input;
        getline(cin, input);
        if (input == "exit")
        {
            break;
        }else{
            string* command = new string;
            string* option = new string;
            string* parameter = new string;
            
            if (!inputHandle(input, command, option, parameter)) {
                print_out("wrong input, try again!\n");
                continue;
            }
            else{
                // path 处理
                FileTreeNode* curFileNode = new FileTreeNode;
                *curFileNode = *fileTree;
                stack<FileTreeNode> fileStack;

                if (!FileCd(curFileNode, parameter, fileStack))
                {
                    print_out("Your Path is Wrong!\n");
                    continue;
                }

                if (*command == "ls")
                {
                    LsHandle(curFileNode, *option=="l", fileStack);
                }else if (*command == "cat")
                {
                    if (*option == "l") { //Ugly
                        print_out("Worng Option!\n" );
                        continue;
                    } else if (curFileNode->isDir) {
                        print_out("Please Enter Right Path!\n");
                    }
                    CatHandle(curFileNode, filestream);
                }
            }  
        }
    }
    
    
    fclose(filestream);
    return 0;
}

void print_out(string str){
    my_print(str.c_str(), str.size());
}

int char2int ( char* source, int begin, int size) {
    int tmp = 0;
    for (int i = begin + size - 1; i >= begin; --i) {
        tmp = tmp * 256 + (unsigned char) source[i];
    }
    return tmp;
}

bool BPBLoad(FILE* filestream, BPB* bpb){
    fread(bpb, 1, 25, filestream);
    BytsPerSec = char2int(bpb->BPB_BytsPerSec,0,2);
    SecPerClus = char2int(bpb->BPB_SecPerClus, 0, 1);
    RsvdSecCnt = char2int(bpb->BPB_RsvdSecCnt, 0, 2);
    FATSz16 = char2int(bpb->BPB_FATSz16, 0, 2);
    NumFATs = char2int(bpb->BPB_NumFATs, 0, 1);
    RootEntCnt = char2int(bpb->BPB_RootEntCnt, 0, 2);
    BytsPerClus = BytsPerSec* SecPerClus;
    FATStart = RsvdSecCnt * BytsPerSec;
    ROOTStart = FATStart + FATSz16 * NumFATs * BytsPerSec;
    DATAStart = ROOTStart + ((32 * RootEntCnt + BytsPerSec - 1) / BytsPerSec)*BytsPerSec;
    return true;
}

bool FileTreeLoad(FILE* filestream, FileTreeNode* fileTree){ //文件树加载,
    
    FileTreeNode* currentFileNode = new FileTreeNode;

    while (true)
    {
        // 读取一个目录项
        RootEntry* rootEntry_ptr = new RootEntry;
        fread(rootEntry_ptr, 1, 32, filestream);
        if(char2int(rootEntry_ptr->DIR_FileSize,0,4) == 0xFFFFFFFF) continue;
        
        int a = char2int(rootEntry_ptr->DIR_Name, 0, 1);
        if (a < '.' || a > 'z') {
            delete(rootEntry_ptr);
            break;
        }
        // child neigh 处理
        FileTreeNode* fileTreeNode = new FileTreeNode;
        if (fileTree->child == nullptr) {
            fileTree->child = fileTreeNode;
        } else {
            currentFileNode->neighbor = fileTreeNode;
        }

        //clus
        int clus = char2int(rootEntry_ptr->DIR_FstClus, 0, 2);
        fileTreeNode->fstClus = clus;
        
        // fileName 处理
        string* fileName = new string;
        for (char ch: rootEntry_ptr->DIR_Name) {
            if (ch == ' ') break;
            fileName->push_back(ch);
        }
        if (rootEntry_ptr->DIR_Attr[0] != 0x10){
            fileName->push_back('.');
            for (int i = 8; i < 11; ++i) {
                char ch = rootEntry_ptr->DIR_Name[i];
                if (ch == ' ') break;
                else fileName->push_back(ch);
            }
        }
        fileTreeNode->fileName = *fileName;
        delete(fileName);
        
        // Dir or File 处理
        if (rootEntry_ptr->DIR_Attr[0] == 0x10) //Dir
        {
            if (!(fileTreeNode->fileName == "." || fileTreeNode->fileName == "..")) {
                fileTree->childDir ++;
                fileTreeNode->isDir = true;
                // 子目录文件树加载
                long tmp = ftell(filestream);
                fseek(filestream, ((clus - 2) * BytsPerClus + DATAStart), SEEK_SET);
                FileTreeLoad(filestream, fileTreeNode);
                fseek(filestream, tmp, SEEK_SET);
            }
        }else{
            fileTree->childFile ++;
            fileTreeNode->fileSize = char2int(rootEntry_ptr->DIR_FileSize,0, 4);
            fileTreeNode->isDir = false;
        }
        
        currentFileNode = fileTreeNode;
        delete(rootEntry_ptr);
    }
    return true;
}

bool inputHandle(string input, string* command, string* option, string* parameter) {
    istringstream iss(input);
    string token;

    while (getline(iss, token, ' '))  {
        if (token=="") continue;
        else if (*command == "") {
            if (token != "ls" && token != "cat") return false;
            *command = token;
        }
        else if (token.substr(0,1) == "-") {
            for (char i: token.substr(1))
            {
                if (i != 'l') return false;
            }
            *option = "l";
        } else if (*parameter == "") {
            *parameter = token;
        } else{
            return false;
        }
    }
    return true;
}

bool FileCd(FileTreeNode* fileNode, string* parameter, stack<FileTreeNode>& fileStack) { //找到对应的目录/文件
    fileStack.push(*fileNode);

    istringstream iss(*parameter);
    string token;

    bool is = false;
    while (getline(iss, token, '/')) {
        // 处理 多个/
        if (token=="") {
            if (is) return false;
            is = true;
            continue;
        }else is = false;

        if (token == ".") continue;
        else if (token == "..") {
            if (fileStack.size() == 1) continue;
            fileStack.pop();
            continue;
        }
        
        FileTreeNode parentNode = fileStack.top();
        FileTreeNode* curNode = parentNode.child;
        
         while (true) {
             if (curNode == nullptr) return false;
             if (curNode->fileName == token)  break;
             curNode = curNode->neighbor;
         }
         
        fileStack.push(*curNode);
    }
    *fileNode = fileStack.top();
    return true;
}

bool CatHandle(FileTreeNode* fileNode, FILE* filestream) {
    if (fileNode->isDir) return false;
    unsigned clus = fileNode->fstClus;
    string content="";

    while (true) {
        // DATA READ
        fseek(filestream, (clus-2)* BytsPerClus+DATAStart, SEEK_SET);
        for (int i = 0; i < BytsPerClus; ++i) {
            char data[1];
            fread(data, 1, 1, filestream);
            if (data[0] == 0x0) break;
            content.push_back(data[0]);
        }

        //FAT12
        char buffer[2];
        unsigned offset = (clus/2)*3+ clus%2;
        fseek(filestream, offset+FATStart, SEEK_SET);
        fread(buffer, 1,2, filestream);

        if (clus%2 == 0) {
            clus = (unsigned char)buffer[0] + ((unsigned char)buffer[1]%16)*256;
        } else{
            clus = (unsigned char)buffer[0]/16 + (unsigned char)buffer[1]*16;
        }
        if (clus == 0xFF7 || clus == 0 || clus == 1) return false;
        if (clus >= 0xFF8) break;
    }
    print_out(content);
    return true;
}

bool LsHandle(FileTreeNode* fileNode, bool option, stack<FileTreeNode>& fileStack) {
    if (!fileNode->isDir) {
        print_out(fileNode->fileName);
        if (option) {
            print_out(" ");
            print_out(to_string(fileNode->fileSize));
        }
        print_out("\n");
        return true;
    }
    pathPrint(fileStack);
    if (option) {
        print_out(" ");
        print_out(to_string(fileNode->childDir));
        print_out(" ");
        print_out(to_string(fileNode->childFile));
    }
    print_out(":\n");

    FileTreeNode* curFileNode = new FileTreeNode;
    curFileNode = fileNode->child;
    while (true) {
        if (curFileNode == nullptr) break;
        if (curFileNode->isDir) print_out("\033[31m");
        print_out(curFileNode->fileName );
        print_out("\033[0m");

        if (!option) print_out("  ");
        else if (curFileNode->fileName == "." || curFileNode->fileName == "..")
            print_out("\n");
        else if (curFileNode->isDir){
            print_out(" ");
            print_out(to_string(curFileNode->childDir));
            print_out(" ");
            print_out(to_string(curFileNode->childFile));
            print_out("\n");
        }
        else {
            print_out(" ");
            print_out(to_string(curFileNode->fileSize));
            print_out("\n");
        }
        curFileNode= curFileNode->neighbor;
    }
    print_out("\n");

    curFileNode = fileNode->child;
    while (true) {
        if (curFileNode== nullptr) break;
        else if(curFileNode->fileName=="." || curFileNode->fileName=="..") {
            curFileNode=curFileNode->neighbor;
            continue;
        }
        if (curFileNode->isDir) {
            fileStack.push(*curFileNode);
            LsHandle(curFileNode, option, fileStack);
            fileStack.pop();
        }
        curFileNode = curFileNode->neighbor;
    }
    return true;
}

bool pathPrint(stack<FileTreeNode> fileStack) { //自栈底向栈顶输出路径
    stack<FileTreeNode> tempStack;
    while (!fileStack.empty()) {
        tempStack.push(fileStack.top());
        fileStack.pop();
    }
    while (!tempStack.empty()) {
        print_out(tempStack.top().fileName);
        print_out("/");
        tempStack.pop();
    }
    return true;
}