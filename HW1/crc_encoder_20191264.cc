#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void crc_encode(char* generator, char* codeword, int data_size, int gen_size, int code_size);

int main(int argc, char **argv) {
    int file_size, dataword_size, codeword_size, gen_size;
    int dataword_cnt, total_encode_size;
    char generator[10];
    char* encoded; char* codeword_stream;

    if (argc != 5) {
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
        return 0;
    }
    FILE* ifp = fopen(argv[1], "rb");
    if (ifp == NULL) {
        printf("input file open error.\n");
        return 0;
    }
    FILE* ofp = fopen(argv[2], "wb");
    if (ofp == NULL) {
        printf("output file open error.\n");
        return 0;
    }
    dataword_size = atoi(argv[4]); //bit
    if (dataword_size != 4 && dataword_size != 8) {
        printf("dataword size must be 4 or 8\n");
        return 0;
    }
    fseek(ifp, 0, SEEK_END);
    file_size = ftell(ifp); //byte
    fseek(ifp, 0, SEEK_SET);

    strcpy(generator, argv[3]);
    gen_size = strlen(generator); //bit
    for (int i = 0; i < gen_size; i++) 
        generator[i] -= '0';
    codeword_size = dataword_size + gen_size - 1; //bit
    dataword_cnt = file_size * 8 / dataword_size; 
    total_encode_size = codeword_size * dataword_cnt / 8; //byte
    if (codeword_size * dataword_cnt % 8 != 0) 
        total_encode_size += 1;
    total_encode_size += 1; //padding bit
    encoded = (char*)malloc(sizeof(char)*total_encode_size);
    memset(encoded,0,total_encode_size);
    codeword_stream = (char*)malloc(sizeof(char)*codeword_size*dataword_cnt);
    memset(codeword_stream, 0, codeword_size*dataword_cnt);
    int idx = 0;
    for (int i = 0; i < file_size; i++) {
        char dataword[8] = {0,};
        char input;
        fread(&input, 1, 1, ifp);
        for (int a = 0; a < 8; a++)
           dataword[7-a] = (input >> a) & 1;
      
        if (dataword_size == 4) {
            for (int j = 0; j < 2; j++) {
                char* codeword = (char*)malloc(sizeof(char)*codeword_size);
                memset(codeword,0,codeword_size);
                memcpy(codeword, dataword+(j*4), 4);
                crc_encode(generator, codeword, dataword_size, gen_size, codeword_size);
                memcpy(codeword_stream + idx, codeword, codeword_size);
                idx += codeword_size;    
                free(codeword);    
            }
        }
        else if (dataword_size == 8) {
            char* codeword = (char*)malloc(sizeof(char)*codeword_size);
            memset(codeword,0,codeword_size);
            memcpy(codeword, dataword, 8);
            crc_encode(generator, codeword, dataword_size, gen_size, codeword_size);
            memcpy(codeword_stream + idx, codeword, codeword_size);
            idx += codeword_size;
            free(codeword);
        }
    } 
    int padding_size = 8 - (codeword_size*dataword_cnt % 8);
    encoded[0] = padding_size;

    unsigned char byte_val = 0;
    for (int i = padding_size; i < 8; i++) {
        byte_val += codeword_stream[i-padding_size];
        if (i != 7) byte_val <<= 1;
    }
    int code_idx = 8 - padding_size;
    encoded[1] = byte_val;
    byte_val = 0;
    for (int i = 2; i < total_encode_size; i++) {
        for (int j = 0; j < 8; j++) {
            byte_val += codeword_stream[code_idx++];
            if (j != 7) byte_val <<= 1;
        }
        encoded[i] = byte_val;
        byte_val = 0;
    }
    fwrite(encoded, total_encode_size, 1, ofp);
    free(encoded);
    free(codeword_stream);
    fclose(ifp);
    fclose(ofp);
    return 0;  
}

 void crc_encode(char* generator, char* codeword, int dataword_size, int gen_size, int codeword_size) {
    char* divided = (char*)malloc(sizeof(char)*codeword_size);
    memset(divided, 0, codeword_size);
    memcpy(divided, codeword, codeword_size);
    for (int i = 0; i < dataword_size; i++) {
        if (divided[i] == 1) {
            for (int j = 0; j < gen_size; j++) {
                if (divided[i+j] == generator[j]) 
                    divided[i+j] = 0;
                else 
                    divided[i+j] = 1;
            }
        }
    }
    //memcpy(codeword + dataword_size, divided + (codeword_size - gen_size), gen_size-1);
    for (int i = dataword_size; i < codeword_size; i++) 
        codeword[i] = divided[i];
    free(divided);
}