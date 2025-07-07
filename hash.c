#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define SEED1    0x12345678
#define SEED2    0x87654321 //nova seed
#include <time.h> // calcular o tempo


typedef struct {
     uintptr_t * table;
     int size; //numero de elemnetos na hash
     int max;   //numero maximo da hash
     uintptr_t deleted; //marcador pra valores apgados
     char * (*get_key)(void *); // get pra chave
     
     float taxa_ocupacao; //adicionado, calcula o quão cheia esta a hash
     int tipo_hash; // 0 = simples e 1 = duplo  
}thash;

uint32_t hashf(const char* str, uint32_t h){
    /* One-byte-at-a-time Murmur hash 
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

// Calcula a posição  na tabela hash
uint32_t _h1(const char *key, int table_size) {
    return hashf(key, SEED1) % table_size;
}
uint32_t _h2(const char *key, int table_size) {
    return (hashf(key, SEED2) % (table_size - 1)) + 1; // já trata para evitar o h2 = 0;
}

int calcular_posicao(thash *h, const char *key, int i) {
    int h1 = _h1(key, h->max);
    int h2 = _h2(key, h->max);
    if (h->tipo_hash == 1)
        return (h1 + i * h2) % h->max;
    else
        return (h1 + i) % h->max;
}

// Insere hash simples e hash dupla (Ajustado)
int hash_insere(thash *h, void *bucket) {

    //trata a taxa de ocupação maxima
    if ((float)(h->size + 1) / h->max > h->taxa_ocupacao) {

        int tam_novo = h->max *2;
        uintptr_t *nova_tabela = calloc(tam_novo, sizeof(void *));
        uintptr_t *tabela_antiga = h->table;
        int max_antigo = h->max;

        h->table = nova_tabela;
        h->max = tam_novo;
        h->size = 0;

        for (int i = 0; i < max_antigo; i++) {
            if (tabela_antiga[i] != 0 && tabela_antiga[i] != h->deleted) {
                void *reg = (void *)tabela_antiga[i];
                if (hash_insere(h, reg) != EXIT_SUCCESS) {
                    free(reg); // Libera se a reinserção falhar
                }
            }
        }

        free(tabela_antiga);
    }

    // Obtemos a chave do bucket
    char *key = h->get_key(bucket);
    
    uint32_t h1 = _h1(key, h->max);
    uint32_t h2 = _h2(key, h->max);


    int pos;
    for (int i = 0; i < h->max; i++) {
        // if (h->tipo_hash == 1)
        //     pos = (h1 + i * h2) % h->max; // Hash duplo
        // else
        //     pos = (h1 + i) % h->max;      // Hash simples

        pos = calcular_posicao(h, key, i);
        //verifica se a posição esta disponivel para inserir  
        if (h->table[pos] == 0 || h->table[pos] == h->deleted) {
            h->table[pos] = (uintptr_t)bucket;
            h->size++;
            return EXIT_SUCCESS;
        }
    }

    // Nenhuma posição disponível
    free(bucket);
    return EXIT_FAILURE;
}

//adicionando o tipo de hash e a taxa de ocupação
int hash_constroi(thash * h,int nbuckets, char * (*get_key)(void *), int tipo_hash, float taxa ){
    h->table =calloc(sizeof(void *),nbuckets+1);
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets+1;
    h->size = 0;
    h->deleted = (uintptr_t)&(h->size);
    h->get_key = get_key;
    h->taxa_ocupacao = taxa;
    h->tipo_hash = tipo_hash;

    return EXIT_SUCCESS;

}

//Busca pra hash simples e dupla
void *hash_busca(thash *h, const char *key) {
    uint32_t h1 = _h1(key, h->max);
    uint32_t h2 = _h2(key, h->max);

    int pos;

    for (int i = 0; i < h->max; i++) {
        pos = calcular_posicao(h, key, i);

        // Se não encontrar nada = encontrou o proxima posição vazia   
        if (h->table[pos] == 0) {
            return NULL; 
        }

        // Se a posição existe algo, verifica se e igual a busca
        if (h->table[pos] != h->deleted) {
            void *reg = (void *)h->table[pos];
            if (strcmp(h->get_key(reg), key) == 0) {
                // printf("Registro Encontrado!!!\n");
                return reg; // achou!
            }
        }
    }
    printf("Registro Não Encontrado!!!\n");

    return NULL; 
}

/* ajustado com o calculo do h1 e h2 */
int hash_remove(thash *h, const char *key) {
    int pos;
    for (int i = 0; i < h->max; i++) {
        pos = calcular_posicao(h, key, i);

        if (h->table[pos] != 0) {
            void *reg = (void *) h->table[pos];
            if (strcmp(h->get_key(reg), key) == 0) {
                h->table[pos] = h->deleted;
                h->size--;
                return EXIT_SUCCESS;
            }  
        } else {
            return EXIT_FAILURE; // não encontrado
        }
    }
    return EXIT_FAILURE;
}

void hash_apaga(thash *h){
    free(h->table);
    h->table = NULL;
    h->max = 0;
    h->size = 0;
}
//Primeira alteração: cep,  nome da cidade e o estado.
typedef struct{
    char  cep_chave[6]; // a chave guarda os 5 primeiros numero do cep
    char cep[10];
    char  cidade[100];
    char  estado[3];
}treg;

char * get_key(void * reg){
    return (*((treg *)reg)).cep_chave;
}

//ajustado
void * aloca_reg(char * nome, char * cep, char * estado){
    treg * reg = malloc(sizeof(treg));
    strcpy(reg->cep,cep);
    strncpy(reg->cep_chave,cep, 5);
    reg->cep_chave[5] = '\0';        
    
    strcpy(reg->cidade,nome);
    strcpy(reg->estado, estado);
    return reg;
}

int insere_ceps(const char *arquivo_csv, thash *h, void **registros, int *total_reg) {
    FILE *fp = fopen(arquivo_csv, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    }

    char linha[256];
    int linha_atual = 0;
    int qtd = 0;

    while (fgets(linha, sizeof(linha), fp)) {
        if (linha_atual++ == 0) continue;

        char *estado = strtok(linha, ";");
        char *cidade = strtok(NULL, ";");
        char *cep = strtok(NULL, ";\n");

        if (estado && cidade && cep) {
            void *reg = aloca_reg(cidade, cep, estado);
            if (hash_insere(h, reg) == EXIT_SUCCESS) {
                if (*total_reg < 10000) {
                    registros[*total_reg] = reg;
                    (*total_reg)++;
                }
                qtd++;
            } else {
                // free(reg); // falha inserção
            }
        }
    }

    fclose(fp);
    return qtd;
}

void realiza_buscas(thash *h, void **registros, int total_reg) {
    int repeticoes = 10000;
    for (int i = 0; i < repeticoes; i++) {
        int idx = i % total_reg;
        char *key = h->get_key(registros[idx]);
        hash_busca(h, key);
    }
}

//MENU pra testes Unitarios
int menu(){
   
    int opcao_percentual;
    printf("Escolha o percentual de insercao:\n");
    for (int i = 1; i <= 9; i++) {
        printf("%d - %d%%\n", i, i * 10);
    }
    printf("0 - 99%%\n");
    scanf("%d", &opcao_percentual);
    printf("Percentual escolhido: %d%%\n", opcao_percentual * 10);


    return opcao_percentual;
}
void libera_registros(void **registros, int total_registros) {
    for (int i = 0; i < total_registros; i++) {
        free(registros[i]);
    }
}

/*FUNÇÕES DE BUSCA- SEGUNDA PARTE*/

void busca10(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.10f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 10%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca20(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.20f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 20%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
    tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca30(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.30f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 30%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca40(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.40f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 40%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca50(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.50f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 50%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca60(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.60f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 60%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca70(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.70f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 70%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca80(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.80f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 80%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca90(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.90f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 90%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}

void busca99(const char *arquivo, int TAM, int tipo_hash) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, TAM, get_key, tipo_hash, 0.99f);

    clock_t i1 = clock();
    int qtd = insere_ceps(arquivo, &h, registros, &total_reg);
    clock_t i2 = clock();

    clock_t b1 = clock();
    realiza_buscas(&h, registros, total_reg);
    clock_t b2 = clock();

    printf("[Hash %d - 99%%] Insercao: %.6f s | Busca: %.6f s | Registros: %d\n",
           tipo_hash, (double)(i2 - i1) / CLOCKS_PER_SEC, (double)(b2 - b1) / CLOCKS_PER_SEC, qtd);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
}
//FUNÇÃO INSERE 4.2
int insere6100(const char *arquivo_csv, int tipo_hash,float taxa) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, 6100, get_key, tipo_hash, taxa);

    clock_t inicio = clock();
    int qtd = insere_ceps(arquivo_csv, &h, registros, &total_reg);
    clock_t fim = clock();

    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    printf("Insere 6100 buckets - Hash %d, Inseridos: %d | Tempo de insercao: %.6f s\n",
           tipo_hash, qtd, tempo);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
    return 0;
}

int insere1000(const char *arquivo_csv, int tipo_hash, float taxa) {
    thash h;
    void *registros[10000];
    int total_reg = 0;

    hash_constroi(&h, 1000, get_key, tipo_hash, taxa); 

    clock_t inicio = clock();
    int qtd = insere_ceps(arquivo_csv, &h, registros, &total_reg);
    clock_t fim = clock();

    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    printf("Insere 1000 buckets - Hash %d, Inseridos: %d | Tempo de insercao: %.6f s\n",
           tipo_hash, qtd, tempo);

    hash_apaga(&h);
    libera_registros(registros, total_reg);
    return 0;
}
void calcula_overhead(double tempo_grande, double tempo_pequeno) {
    double overhead = ((tempo_pequeno - tempo_grande) / tempo_grande) * 100.0;
    printf("Overhead da tabela menor: %.2f%%\n", overhead);
}
void insere_taxas(const char *arquivo_csv, int tipo_hash) {
    float taxas[] = {0.10f, 0.20f, 0.30f, 0.40f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f, 0.99f};
    int num_taxas = sizeof(taxas) / sizeof(taxas[0]);

    for (int i = 0; i < num_taxas; i++) {
        float taxa = taxas[i];

        printf("Comparando insercoes para %.0f%% de ocupacao (Hash %d)\n", taxa * 100, tipo_hash);

        clock_t inicio_6100 = clock();
        insere6100(arquivo_csv, tipo_hash, taxa);
        clock_t fim_6100 = clock();
        double tempo_6100 = (double)(fim_6100 - inicio_6100) / CLOCKS_PER_SEC;

        clock_t inicio_1000 = clock();
        insere1000(arquivo_csv, tipo_hash, taxa);
        clock_t fim_1000 = clock();
        double tempo_1000 = (double)(fim_1000 - inicio_1000) / CLOCKS_PER_SEC;

        calcula_overhead(tempo_6100, tempo_1000);
        printf("comparacao %.0f%%\n\n", taxa * 100);
    }
}
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Uso: %s <arquivo_csv>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *arquivo_csv = argv[1];
    int tamanho_base = 6100;
   
    int tipo_hash;
    int opcao_percentual;
    int percentual = 10; 

    printf("Escolha o tipo de hash:\n");
    printf("0 - Hash Simples\n");
    printf("1 - Hash Dupla\n");
    printf("2 - Executar ambas\n");
    printf("Opcao: \n");
    scanf("%d", &tipo_hash);
    //Cria e testa a hash
    thash h;

    if (tipo_hash == 0 || tipo_hash == 1) {
        int opcao_percentual = menu(); 
        float taxa;
        if (opcao_percentual == 1) {
            taxa = 0.1;
            busca10(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 2) {
            taxa = 0.2;
            busca20(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 3) {
            taxa = 0.3;
            busca30(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 4) {
            taxa = 0.4;
            busca40(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 5) {
            taxa = 0.5;
            busca50(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 6) {
            taxa = 0.6;
            busca60(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 7) {
            taxa = 0.7;
            busca70(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 8) {
            taxa = 0.8;
            busca80(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 9) {
            taxa = 0.9;
            busca90(arquivo_csv, 6100, tipo_hash);
        } else if (opcao_percentual == 0) {
            taxa = 0.99;
            busca99(arquivo_csv, 6100, tipo_hash);
        }
        printf("\n---------\n");
    }
    if (tipo_hash == 2) {
        for (int tipo = 0; tipo <= 1; tipo++) {
            busca10(arquivo_csv, 6100, tipo);
            busca20(arquivo_csv, 6100, tipo);
            busca30(arquivo_csv, 6100, tipo);
            busca40(arquivo_csv, 6100, tipo);
            busca50(arquivo_csv, 6100, tipo);
            busca60(arquivo_csv, 6100, tipo);
            busca70(arquivo_csv, 6100, tipo);
            busca80(arquivo_csv, 6100, tipo);
            busca90(arquivo_csv, 6100, tipo);
            busca99(arquivo_csv, 6100, tipo);
            printf("\n------Comparacao entre tabela 6100 e 1000 buckets------\n");
            insere_taxas(arquivo_csv, tipo);
        }        
    }

    return 0;
}
