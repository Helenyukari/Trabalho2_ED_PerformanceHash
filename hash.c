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
        // Nova tabela
         uintptr_t *nova_tabela = calloc(tam_novo, sizeof(void *));
        // Guarda os dados antigos
        uintptr_t *tabela_antiga = h->table;
        int max_antigo = h->max;

        h->table = nova_tabela;
        h->max = tam_novo;
        h->size = 0;

        for (int i = 0; i < max_antigo; i++) {
            if (tabela_antiga[i] != 0 && tabela_antiga[i] != h->deleted) {
                void *reg = (void *)tabela_antiga[i];
                hash_insere(h, reg);  // reinsere o dado na nova tabela
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
    uint32_t h1 = _h1(key, h->max);
    uint32_t h2 = _h2(key, h->max);

   int pos;
    for (int i = 0; i < h->max; i++) {
        // if (h->tipo_hash == 1)
        //     pos = (h1 + i * h2) % h->max; // hash dupla
        // else
        //     pos = (h1 + i) % h->max;      // hash simples
        pos = calcular_posicao(h, key, i);

        if (h->table[pos] != 0) {
            void *reg = (void *) h->table[pos];
            if (strcmp(h->get_key(reg), key) == 0) {
                free(reg);
                h->table[pos] = h->deleted;
                h->size--;
                return EXIT_SUCCESS;
            }  
        }
        else{
            return EXIT_FAILURE; // trata a colisão, se estiver vazia não encontrou
        }
    }

    return EXIT_FAILURE;
}

/*não alterado*/
void hash_apaga(thash *h){
    int pos;
    for(pos =0;pos< h->max;pos++){
        if (h->table[pos] != 0){
            if (h->table[pos]!=h->deleted){
                free((void *)h->table[pos]);
            }
        }
    }
    free(h->table);
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

//LÊ o Arquivo e guarda na hash --> modifiquei e coloquei pra busca tmb
int carregar_ceps(const char *arquivo_csv, thash *h) {
    FILE *fp = fopen(arquivo_csv, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    }

    char linha[256];
    int linha_atual = 0;
    int qtd = 0; //aqui vamos contar qtd de registros inseridos

    // Lista dos CEPs inseridos para depois buscar
    char ceps_para_buscar[10000][6]; 
    int total_ceps = 0;

    clock_t inicio_insercao = clock();

    while (fgets(linha, sizeof(linha), fp)) {
        if (linha_atual++ == 0) continue; // pula cabeçalho

        char *estado = strtok(linha, ";");
        char *cidade = strtok(NULL, ";");
        char *cep = strtok(NULL, ";\n");

        if (estado && cidade && cep) {
            void *reg = aloca_reg(cidade, cep, estado);
            hash_insere(h, reg);

            if (total_ceps < 10000) {
                strncpy(ceps_para_buscar[total_ceps], cep, 5);
                ceps_para_buscar[total_ceps][5] = '\0';
                total_ceps++;
            }

            qtd++;
        }

        // if (limite > 0 && qtd >= limite) break;
    }

    clock_t fim_insercao = clock();
    fclose(fp);

    double tempo_insercao = (double)(fim_insercao - inicio_insercao) / CLOCKS_PER_SEC;

    // --------------------- Busca --------------------
    clock_t inicio_busca = clock();

    int repeticoes = 10000;
    for (int i = 0; i < repeticoes; i++) {
        int indice = i % total_ceps;
        hash_busca(h, ceps_para_buscar[indice]);
    }

    clock_t fim_busca = clock();
    double tempo_busca = (double)(fim_busca - inicio_busca) / CLOCKS_PER_SEC;

    // --------------------- Saída --------------------
    printf("Tempo de insercao: %f segundos\n", tempo_insercao);
    printf("Tempo de busca:    %f segundos\n", tempo_busca);
    printf("Quantidade de elementos inseridos: %d\n", qtd);

    return EXIT_SUCCESS;
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

/*FUNÇÕES DE BUSCA- SEGUNDA PARTE*/
void busca10 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.10f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca20 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.20f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca30 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.30f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca40 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.40f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca50 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.50f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca60 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.60f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca70 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.70f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca80 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.80f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca90 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.90f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
}
void busca99 (const char *arquivo, int TAMANHO_TABELA,int tipo_hash) {
    thash h;
    hash_constroi(&h, TAMANHO_TABELA, get_key, tipo_hash, 0.99f);
    carregar_ceps(arquivo, &h);
    hash_apaga(&h);
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
        opcao_percentual = menu();
        hash_constroi(&h, tamanho_base, get_key, tipo_hash, opcao_percentual);

    }
    else if( tipo_hash == 2){
        hash_constroi(&h, tamanho_base, get_key, tipo_hash, opcao_percentual);

    }

    carregar_ceps(arquivo_csv, &h);
    


   
    hash_apaga(&h);

    return 0;
}
