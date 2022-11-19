#include <iostream>
#include "bmpNew.h"
#include <Eigen/Dense>
#include <Eigen/LU>
#include <cmath>
#include <vector>
#include <fstream>
#include <string.h>
#include <algorithm>

using namespace Eigen;
using namespace std;

unsigned int* macroarray[32][32];
unsigned char* zigzagarray[32][32];

signed char DPCMarray[32 * 32];
signed char DPCMdiffrential[32 * 32];
const int INDEXNUMBER = 256;
const float pi = 3.141592;
unsigned int quantizetable[64] = {16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99};
static int bits = 0;
vector<int> AChuffarray;



class AChuff
{
public:
    int Amp;
    int frequency=0;
    int ACbits = 0;
    AChuff* leftChild;
    AChuff* rightChild;
    string code;
};

vector<AChuff>kkk;
vector<AChuff>table;

AChuff extractMin()
{
    unsigned int min = UINT_MAX;
    vector<AChuff>::iterator iter, position;
    for (iter = kkk.begin(); iter != kkk.end(); iter++)
    {
        if (min > (*iter).frequency)
        {
            position = iter;
            min = (*iter).frequency;
        }
    }

    AChuff tempNode = (*position);
    kkk.erase(position);

    return tempNode;
}

AChuff getHuffmanTree()
{
    while (!kkk.empty())
    {
        AChuff* tempNode = new AChuff;
        AChuff* tempNode1 = new AChuff;
        AChuff* tempNode2 = new AChuff;
        *tempNode1 = extractMin();
        *tempNode2 = extractMin();

        tempNode->leftChild = tempNode1;
        tempNode->rightChild = tempNode2;
        tempNode->frequency = tempNode1->frequency + tempNode2->frequency;
        kkk.push_back(*tempNode);

        //Root Node만 남았으므로 Huffman Tree 완성 
        if (kkk.size() == 1)
            break;
    }

    return kkk[0];
}

void depthFirstSearch(AChuff* tempRoot, string s)
{
    AChuff* root1 = new AChuff;
    root1 = tempRoot;

    root1->code = s;
    if (root1 == NULL)
    {

    }
    else if (root1->leftChild == NULL && root1->rightChild == NULL)
    {
        cout << "\t" << root1->Amp << "\t" << root1->code << endl;
        table.push_back(*tempRoot);
    }
    else
    {
        root1->leftChild->code = s.append("0");


        s.erase(s.end() - 1);
        
        root1->rightChild->code = s.append("1");


        s.erase(s.end() - 1);

        depthFirstSearch(root1->leftChild, s.append("0"));
        s.erase(s.end() - 1);
        depthFirstSearch(root1->rightChild, s.append("1"));
        s.erase(s.end() - 1);
    }
}

class DPCMCode
{
private:
    //만든사람: 오명훈
public:

    signed char size;
    signed char amp;
    string sizecode[9] = { "00","010","011","100","101","110","1110","11110","111110" };
    string sizehuffcode;
    string amphuffcode;
    string huffcode;


    DPCMCode()
    {
        this->amp = 0;
        this->sizehuffcode = sizecode[0];
    }
    void getsize(signed char x)
    {
        this->amp = x;
    }
    void sizehuffman()
    {
        int tempamp = this->amp;
        for (int i = 0; i < 11; i++)
        {
            int a = pow(2, i);
            int b = pow(2, i + 1) - 1;

            if (a <= abs(tempamp) && abs(tempamp) <= b)
            {
                this->size = i + 1;
            }
            if (abs(tempamp) == 0)
            {
                this->size = 0;
            }
        }
        for (int i = 0; i < 9; i++)
        {
            this->sizehuffcode = sizecode[this->size];
        }
        if (this->size == 0)
        {
            bits = bits + 2;
        }
        if (this->size >=1 and this->size <=5)
        {
            bits = bits + 3;
        }
        if (this->size ==6)
        {
            bits = bits + 4;
        }
        if (this->size == 7)
        {
            bits = bits + 5;
        }
        if (this->size == 8)
        {
            bits = bits + 6;
        }
        
    }
    void amphuffman()
    {
        vector<int> nodeArray;
        int a = this->size;
        int ampbinary;
        for (int i = -pow(2, a) + 1; i <= -pow(2, a - 1); i++)
        {
            nodeArray.push_back(i);
        }
        for (int i = pow(2, a - 1); i <= pow(2, a) - 1; i++)
        {
            nodeArray.push_back(i);
        }
        for (int i = 0; i < pow(2, a); i++)
        {
            if (nodeArray[i] == this->amp)
            {
                ampbinary = i;
            }
        }
        string s = "";

        if (ampbinary == 0)
        {
            this->amphuffcode = "0";
        }
        else
        {
            while (ampbinary > 0)
            {
                if (ampbinary % 2 == 1)
                {
                    s = "1" + s;
                }
                else
                {
                    s = "0" + s;
                }
                ampbinary >>= 1;
                bits++;
            }
            this->amphuffcode = s;
        }
        
    }
    void gethuffmanCode()
    {
        sizehuffman();
        amphuffman();
        this->huffcode = this->sizehuffcode + this->amphuffcode;
    }
};
void RLC(int i,int j)
{

    vector <int> ACarray;
    int zerocount = 0;
    int length=0;


    for (int k = 1; k < 64; k++)
    {
        if (zigzagarray[i][j][k] != 0)
        {
            ACarray.push_back(zerocount);
            ACarray.push_back(zigzagarray[i][j][k]);
            zerocount = 0;
        }
        if (zerocount < 16 &&( zigzagarray[i][j][k] ==0 && k != 63))
        {
            zerocount++;
            
        }
        if( zerocount==16 && (zigzagarray[i][j][k]==0 && k!= 63))
        {
            ACarray.push_back(zerocount-1);

            ACarray.push_back(0);
            zerocount = 0;
        }
        if (k == 63)
        {
            ACarray.push_back(zerocount);
            ACarray.push_back(0);
            ACarray.push_back(0);
            ACarray.push_back(0);

        }
 
    }
    for (vector<int>::iterator iter = ACarray.begin(); iter != ACarray.end(); iter++)
    {

        AChuffarray.push_back(*iter);

    }


}
void DPCM()
{
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (i == 0 && j == 0)
            {
                DPCMarray[0] = zigzagarray[0][0][0];
            }
            else
            {
                DPCMarray[32 * i + j] = zigzagarray[j][i][0];
            }

        }
    }
    for (int i = 0; i < (32 * 32 - 1); i++)
    {
        DPCMdiffrential[i + 1] = DPCMarray[i] - DPCMarray[i + 1];
    }
    DPCMdiffrential[0] = DPCMarray[0];
}

void ZigZagscan(int i,int j,unsigned int* k)
{
    unsigned char* scan = new unsigned char[64];

    scan[0] = k[0];

    scan[1] = k[1];
    scan[2] = k[8];
    scan[3] = k[16];
    scan[4] = k[9];
    scan[5] = k[2];
    scan[6] = k[3];
    scan[7] = k[10];
    scan[8] = k[17];
    scan[9] = k[24];
    scan[10] = k[32];
    scan[11] = k[25];
    scan[12] = k[18];
    scan[13] = k[11];
    scan[14] = k[4];
    scan[15] = k[5];
    scan[16] = k[12];
    scan[17] = k[19];
    scan[18] = k[26];
    scan[19] = k[33];
    scan[20] = k[40];
    scan[21] = k[48];
    scan[22] = k[41];
    scan[23] = k[34];
    scan[24] = k[27];
    scan[25] = k[20];
    scan[26] = k[13];
    scan[27] = k[6];
    scan[28] = k[7];
    scan[29] = k[14];
    scan[30] = k[21];
    scan[31] = k[28];
    scan[32] = k[35];
    scan[33] = k[42];
    scan[34] = k[49];
    scan[35] = k[56];
    scan[36] = k[57];
    scan[37] = k[50];
    scan[38] = k[43];
    scan[39] = k[36];
    scan[40] = k[29];
    scan[41] = k[22];
    scan[42] = k[15];
    scan[43] = k[23];
    scan[44] = k[30];
    scan[45] = k[37];
    scan[46] = k[44];
    scan[47] = k[51];
    scan[48] = k[58];
    scan[49] = k[59];
    scan[50] = k[52];
    scan[51] = k[45];
    scan[52] = k[38];
    scan[53] = k[31];
    scan[54] = k[39];
    scan[55] = k[46];
    scan[56] = k[53];
    scan[57] = k[60];
    scan[58] = k[61];
    scan[59] = k[54];
    scan[60] = k[47];
    scan[61] = k[55];
    scan[62] = k[62];
    scan[63] = k[63];

    zigzagarray[i][j] = scan;
}

void Quantization(unsigned int* k)
{
    for (int i = 0; i < 64; i++)
    {
        k[i] = round((k[i] / quantizetable[i]));
    }
}



void Furiermacroarray(unsigned int* k)
{
    float TempArr[64];

    for (int yindex=0; yindex<8;yindex++)
    {
        for (int xindex = 0; xindex < 8; xindex++)
        {
            float sum = 0;
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    if (xindex != 0 && yindex != 0)
                    {
                        sum =sum+ cos(((2*j + 1) * xindex * pi) / 16) * cos(((2*i + 1) * yindex * pi) / 16) * k[j + 8 * i];
                       

                    }
                    if ((xindex==0 || yindex == 0 ) && (((xindex ==0)&&(yindex ==0)))!=1)
                    {
                        sum =sum+ (cos(((2 * j + 1) * xindex * pi) / 16) * cos(((2 * i + 1) * yindex * pi) / 16) * k[j + 8 * i]);

                    }
                    if (xindex==0 && yindex == 0)
                    {
                        sum =sum+((cos(((2 * j + 1) * xindex * pi) / 16) * cos(((2 * i + 1) * yindex * pi) / 16) )* k[j + 8 * i]);

                        
                        
                        
                      
                    }
                }
            }
            if ((xindex == 0 || yindex == 0) && (((xindex == 0) && (yindex == 0))) != 1)
            {
                sum = sum * (1 / sqrt(2));
            }
            if (xindex == 0 && yindex == 0)
            {
 
                sum = sum /2;

            }
            TempArr[yindex * 8 + xindex] = sum;


        }

    
    }


    for (int i = 0; i < 64; i++)
    {
        k[i] = abs(TempArr[i]);
    }

}

//Macro Block 을 만들어주는 코드입니다. 원하는 파일(레나) 를 입력하고, 원하는 매크로블록의 연산을 만들어낼 수 있습니다.
void macroblock(int i, int j,unsigned char *k)
{
    unsigned int* mbpointarray = new unsigned int[64];
    int iter = 0;
    for (int yindex = 0; yindex < 8; yindex++)
    {
        for (int xindex = 0; xindex < 8; xindex++)
        {
            mbpointarray[iter] = k[8 * i + 256 * j * 8+xindex+256*yindex];
            iter++;
        }
    }
    macroarray[i][j] = mbpointarray;

}

int main()
{
    
    int x, y;
    unsigned char* lena = ReadBmp("Lena.bmp", &x, &y);

    int width = 256;
    int height = 256;
    unsigned char mylena[256*256];
    for (int i = 0; i < (x*y); i++)
    {
        mylena[i] = lena[i];
    }

    WriteBmp("mylena.bmp", mylena,x,y);

    macroblock(0, 0, mylena);
    //cout <<endl<<static_cast<int>( mylena[256*255+1])<<endl;
    //cout << endl<<static_cast<int>(macroarray[0][31][57]);
    //매크로블록 좌표 (0,1)의 (0,1)블록은 mylena[256*8 +1]과 같습니다.
    // 매크로 블록 (0,31)번의 (7,1) 블록은 매크로 블록 내부 어레이의 57번쨰 인덱싱입니다.
    //검증 완료.
    
    cout << "\n매크로블록 (10,10) 번째의 내부 값 :\n" << endl;
    for (int yindex = 0; yindex < 8; yindex++)
    {
        for (int xindex = 0; xindex < 8; xindex++)
        {
            cout << static_cast<float>(macroarray[0][0][yindex * 8 + xindex]) << "  ";
        }
        cout << endl;
    }
    cout<<"\n----------------------\n퓨리에 변환 값 \n"<<endl;
    Furiermacroarray(macroarray[0][0]);
    for (int yindex = 0; yindex < 8; yindex++)
    {
        for (int xindex = 0; xindex < 8; xindex++)
        {
            cout << static_cast<float>(macroarray[0][0][yindex * 8 + xindex])<<"  ";
        }
        cout << endl;
    }
    Quantization(macroarray[0][0]);
    cout << "\n-------------------------------\n 양자화 값 \n";

    for (int yindex = 0; yindex < 8; yindex++)
    {
        for (int xindex = 0; xindex < 8; xindex++)
        {
            cout << static_cast<float>(macroarray[0][0][yindex * 8 + xindex]) << "  ";
        }
        cout << endl;
    }
    cout << "\n-------------------------------------\n 지그재그 값 \n";
    ZigZagscan(0,0,macroarray[0][0]);
    for (int i = 0; i < 64; i++)
    {
        cout << static_cast<int>(zigzagarray[0][0][i]) << "   ";
    }
    //전체 매크로블록(이미지)에 적용
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            macroblock(i, j, mylena);
            Furiermacroarray(macroarray[i][j]);
            Quantization(macroarray[i][j]);
            ZigZagscan(i, j, macroarray[i][j]);
        }
    }
    cout << "\n-------------------------------\n 전체 DPCM \n";
    DPCM();
    for (int i = 0;i< sizeof(DPCMdiffrential); i++)
    {
        cout <<static_cast<int>( DPCMdiffrential[i]) << "    ";
    }
    cout << "\n\nDPCM 허프만코드 --------------------------------\n\n\n";

    ofstream Good;
    Good.open("Experience.txt", ios::out | ios::binary);
    DPCMCode DChuffmancode[32 * 32];
    for (int i = 0; i < 32 * 32; i++)
    {
        DChuffmancode[i] = DPCMCode();
        DChuffmancode[i].getsize(DPCMdiffrential[i]);
        cout << static_cast<int>(DChuffmancode[i].amp) << "\t";
        DChuffmancode[i].gethuffmanCode();

        cout << static_cast<int>(DChuffmancode[i].size) << "\t" << DChuffmancode[i].huffcode << endl;
        Good << DChuffmancode[i].huffcode;

        
    }

    cout << "\n--------------------------------------\nDPCM BITS :  " << bits << endl;

    for (int m = 0; m < 32; m++)
    {
        for (int h = 0; h < 32; h++)
        {
            RLC(m, h);
        }
    }

    vector<int>::iterator iter,iterin;
    vector<int>AChuffarraycopy;
    for ( iter = AChuffarray.begin(); iter != AChuffarray.end(); iter++)
    {
        AChuffarraycopy.push_back(*iter);
    }
    
    sort(AChuffarraycopy.begin(), AChuffarraycopy.end());
    AChuffarraycopy.erase(unique(AChuffarraycopy.begin(), AChuffarraycopy.end()), AChuffarraycopy.end());


    for (iter = AChuffarraycopy.begin(); iter != AChuffarraycopy.end(); iter++)
    {
        AChuff tempnode;
        tempnode.Amp = *iter;
        for (iterin = AChuffarray.begin(); iterin != AChuffarray.end(); iterin++)
        {
            if (*iter == *iterin)
            {
                tempnode.frequency++;
            }
        }
        kkk.push_back(tempnode);
    }
    cout << "\n-------------AC huff coding: Amp , frequency, code ----------\n";

    AChuff root = getHuffmanTree();
    depthFirstSearch(&root, "");

    for (vector<AChuff>::iterator a = table.begin(); a != table.end(); a++)
    {
        cout << (*a).Amp << "   " << (*a).frequency <<"  "<<(*a).code<<"  " <<(*a).ACbits<<endl;
    }
    cout << "\n---------------\n" << endl;
    for ( iter = AChuffarray.begin(); iter != AChuffarray.end(); iter++)
    {
        for (vector<AChuff>::iterator qq = table.begin(); qq != table.end(); qq++)
        {
            if (*iter == (*qq).Amp)
            {
                cout << (*qq).code << " ";
                Good << (*qq).code;
            }
        }

    }
}