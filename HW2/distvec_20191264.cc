#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_VERTICES 100
#define MAX_LINE 1000
#define INF 999
#define TRUE 1
#define FALSE 0


int num_v;
int cost[MAX_VERTICES][MAX_VERTICES];
int distance[MAX_VERTICES][MAX_VERTICES];
int next[MAX_VERTICES][MAX_VERTICES];

void routing_table();
void write_message(char* argv, FILE *wfp);

int main(int argc, char** argv) {
    int i, j;

    if (argc != 4) {
        printf("usage : distev topologyfile messagefile changesfile\n");
        return 0;
    }
    FILE *tfp = fopen(argv[1], "r"); //topologyfile 
    if (tfp == NULL) {
        printf("Error: open input file\n");
        return 0;
    }
    FILE *mfp = fopen(argv[2], "r"); //messagefile
    if (mfp == NULL) {
        printf("Error: open input file\n");
        return 0;
    }
    fclose(mfp);
    FILE *cfp = fopen(argv[3], "r"); //changefile
    if (cfp == NULL) {
        printf("Error: open input file\n");
        return 0;
    }
    fscanf(tfp, "%d", &num_v);
    for (i = 0; i < num_v; i++) 
        for (j = 0; j < num_v; j++)
            cost[i][j] = INF;
    int tmp;
    while (fscanf(tfp, "%d %d %d", &i, &j, &tmp) != EOF) {
        cost[i][j] = tmp;
        cost[j][i] = tmp;
    }
    for (i = 0; i < num_v; i++) 
        for (j = 0; j < num_v; j++) {
            distance[i][j] = INF;
            next[i][j] = -1;
        }
    routing_table();
    
    FILE *wfp = fopen("output_dv.txt", "w"); //output file
    if (wfp == NULL) {
        printf("Error: open output file\n");
        return 0;
    }
    for (i = 0; i < num_v; i++) {
        for (j = 0; j < num_v; j++) {
            if (distance[i][j] == 999) continue;
            fprintf(wfp, "%d %d %d\n", j, next[i][j], distance[i][j]);
        }
        fprintf(wfp,"\n");
    }
    write_message(argv[2], wfp);
    /*for (i = 0; i < num_v; i++) {
        for (j = 0; j < num_v; j++) {
            printf("%d ",next[i][j]);
        }
        printf("\n");
    }*/
    int n, m, chcost;
    while (1) {
        if (fscanf(cfp, "%d %d %d", &n, &m, &chcost) == EOF) break;
        if (chcost < 0) chcost = 999;
        cost[n][m] = chcost; cost[m][n] = chcost;
        for (i = 0; i < num_v; i++) 
            for (j = 0; j < num_v; j++) {
                distance[i][j] = INF;
                next[i][j] = -1;
        }
        routing_table();
        for (i = 0; i < num_v; i++) {
            for (j = 0; j < num_v; j++) {
                if (distance[i][j] == 999) continue;
                fprintf(wfp, "%d %d %d\n", j, next[i][j], distance[i][j]);
            }
            fprintf(wfp,"\n");
        }
        write_message(argv[2], wfp);
    }  
    printf("Complete. Output file written to output_dv.txt\n");   
    fclose(tfp);
    fclose(cfp);
    fclose(wfp);
    return 0;
}

void routing_table() {
    int i, j, k;
    int updated = TRUE;
    for (i = 0; i < num_v; i++) {
        for (j = 0; j < num_v; j++) {
            if (i == j) {
                distance[i][j] = 0;
                next[i][j] = i;
            }
            else if (cost[i][j] < INF) {
                distance[i][j] = cost[i][j];
                next[i][j] = j;
            }
            else {
                distance[i][j] = INF;
                next[i][j] = -1;
            }
        }
    }
    while (updated) {
        updated = FALSE;
        for (i = 0; i < num_v; i++) {
            for (j = 0; j < num_v; j++) {
                if (i != j && cost[i][j] != INF) {
                    for (k = 0; k < num_v; k++) {
                        if (distance[i][k] > distance[i][j] + distance[j][k]) {
                            distance[i][k] = distance[i][j] + distance[j][k];
                            next[i][k] = next[i][j];
                            updated = TRUE;
                        }
                        else if (distance[i][k] == distance[i][j] + distance[j][k]) {
                            if (next[i][k] == -1 || next[i][j] < next[i][k]) {
                                next[i][k] = next[i][j];
                                updated = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
}

void write_message(char* argv, FILE *wfp) {
    FILE *mfp = fopen(argv, "r");
    int start, end;
    char strtmp;
    char message[MAX_LINE];
    int idx;
    while (1) {
        strtmp = fgetc(mfp);
        if (strtmp == EOF) break;
        start = atoi(&strtmp); fgetc(mfp);
        strtmp = fgetc(mfp);
        end = atoi(&strtmp); fgetc(mfp);
        idx = 0;
        while (1) {
            strtmp = fgetc(mfp);
            if (strtmp == EOF || strtmp == '\n') {
                message[idx] = '\0'; break;
            }
            message[idx++] = strtmp;
        }
        if (distance[start][end] != 999) {
            fprintf(wfp, "from %d to %d cost %d hops ", start, end, distance[start][end]);
            while (1) {
                fprintf(wfp, "%d ", start);
                start = next[start][end];
                if (start == end) break;
            }
            fprintf(wfp, "message %s\n", message);
        }
        else {
            fprintf(wfp, "from %d to %d cost infinite hops unreachable message %s\n", start, end, message);
        }
    }
    fprintf(wfp, "\n");
    fclose(mfp);
}
