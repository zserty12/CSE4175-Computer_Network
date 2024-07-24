#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void crc_decode(char* generator, char* codeword, int dataword_size, int gen_size, char codeword_size, int* err_num);

int main(int argc, char **argv) {
    int file_size, dataword_size, codeword_size, gen_size;
    int dataword_cnt, codeword_cnt, total_decoded_size;
    char generator[10];
    char* encoded_stream; char* codeword_stream;
    char* decoded; char* dataword;
    int err_num = 0;

    
    if (argc != 6) {
        printf("usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        return 0;
    }
    FILE* ifp = fopen(argv[1], "rb");
    if (ifp == NULL) {
        printf("input file open error\n");
        return 0;
    }
    FILE* ofp = fopen(argv[2], "wb");
    if (ofp == NULL) {
        printf("output file open error\n");
        return 0;
    }
    FILE* rfp = fopen(argv[3], "w");
    if (rfp == NULL) {
        printf("result file open error\n");
        return 0;
    }
    strcpy(generator, argv[4]);
    gen_size = strlen(generator); //bit
    for (int i = 0; i < gen_size; i++)
        generator[i] -= '0';
    dataword_size = atoi(argv[5]); //bit
    if (dataword_size != 4 && dataword_size != 8) {
        printf("dataword size must be 4 or 8\n");
        return 0;
    }
    fseek(ifp, 0, SEEK_END);
    file_size = ftell(ifp); //byte
    fseek(ifp, 0, SEEK_SET);

    encoded_stream = (char*)malloc(sizeof(char)*(file_size*8));
    memset(encoded_stream, 0, file_size*8);

    int padding_size;
    
    for (int i = 0; i < file_size; i++) {
        char input;
        fread(&input, 1, 1, ifp);
        if (i == 0) padding_size = input;
        for (int j = 0; j < 8; j++) 
            encoded_stream[i*8 + j] = (input >> 7-j) & 1;
    }
    codeword_size = dataword_size + gen_size - 1; //bit
    codeword_cnt = ((file_size * 8) -  (8 + padding_size)) / codeword_size; 
    codeword_stream = (char*)malloc(sizeof(char)*codeword_size*codeword_cnt);
    memset(codeword_stream, 0, codeword_cnt*codeword_size);
    int idx = 8 + padding_size;
    memcpy(codeword_stream, encoded_stream + idx, codeword_cnt*codeword_size);
    if (dataword_size == 4)
        dataword_cnt = codeword_cnt / 2;
    else dataword_cnt = codeword_cnt;
    dataword = (char*)malloc(sizeof(char)*dataword_cnt*8);
    memset(dataword, 0, dataword_cnt*8);
    decoded = (char*)malloc(sizeof(char)*dataword_cnt);
    memset(decoded, 0, dataword_cnt);
    int code_idx = 0; int data_idx = 0;
    for (int i = 0; i < codeword_cnt; i++) {
        char* codeword = (char*)malloc(sizeof(char)*codeword_size);
        memset(codeword, 0, codeword_size);
        memcpy(codeword, codeword_stream + code_idx, codeword_size);
        crc_decode(generator, codeword, dataword_size, gen_size, codeword_size, &err_num);
        memcpy(dataword + data_idx, codeword, dataword_size);
        code_idx += codeword_size; data_idx += dataword_size;
        free(codeword);
    } 
    data_idx = 0;
    unsigned char byte_val = 0;
    for (int i = 0; i < dataword_cnt; i++) {
        for (int j = 0; j < 8; j++) {
            byte_val += dataword[data_idx++];
            if (j != 7) byte_val <<= 1;
        }
        decoded[i] = byte_val;
        byte_val = 0;
    }
    fwrite(decoded, dataword_cnt, 1, ofp);   
    fprintf(rfp, "%d %d", codeword_cnt, err_num);
    free(decoded);
    free(dataword);
    fclose(ifp);
    fclose(ofp);
    return 0;
}

void crc_decode(char* generator, char* codeword, int dataword_size, int gen_size, char codeword_size, int* err_num) {
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
    for (int j = 0; j < codeword_size; j++) {
        if (divided[j] == 1) {
            *err_num += 1;
            break;
        }
    }
    free(divided);
}