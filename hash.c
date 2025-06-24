#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define SEED1    0x12345678
#define SEED2    0x87654321 //nova seed
#include <time.h>


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
        // if (h->tipo_hash == 1)
        //     pos = (h1 + i * h2) % h->max;  // hash duplo
        // else
        //     pos = (h1 + i) % h->max;       // hash simples
        pos = calcular_posicao(h, key, i);

        // Se não encontrar nada = encontrou o proxima posição vazia   
        if (h->table[pos] == 0) {
            return NULL; 
        }

        // Se a posição existe algo, verifica se e igual a busca
        if (h->table[pos] != h->deleted) {
            void *reg = (void *)h->table[pos];
            if (strcmp(h->get_key(reg), key) == 0) {
                printf("Registro Encontrado!!!\n");
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


//Lê o csv e inserie na hash
int carregar_ceps(const char *arquivo_csv, thash *h, int limite) {
    FILE *fp = fopen(arquivo_csv, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    }

    char linha[256];
    int linha_atual = 0;
    int qtd = 0;

    clock_t inicio = clock();

    while (fgets(linha, sizeof(linha), fp)) {
        if (linha_atual++ == 0) {
            continue; // Pula o cabeçalho
        }
        //strtok pega um pedaço do texto
        char *estado = strtok(linha, ";");
        char *cidade = strtok(NULL, ";");
        char *cep = strtok(NULL, ";\n");

        if (estado && cidade && cep) {
            void *reg = aloca_reg(cidade, cep, estado);
            qtd++;
            hash_insere(h, reg);
        }

        // Limite de inserção
        if (limite > 0 && qtd >= limite) {
            break;
        }
    }

    clock_t fim = clock();
    fclose(fp);
    //transforma o tempo medido em segundos
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    printf("Tempo de insercao: %f segundos\n", tempo);
    printf("Quantidade de elementos %d\n", qtd);

    return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {
        // teste_busca_simples();

    if (argc < 2) {
        printf("Uso: %s <arquivo_csv>\n", argv[0]);
        return EXIT_FAILURE;
    }
    //aqui o codigo salva a quantidade de entradas
    int limite = 0;
    if (argc >= 3) {
       limite = atoi(argv[2]); // converte argumento para inteiro
    }


   
    const char *arquivo_csv = argv[1]; // salva em um ponteiro o nome do csv .\Lista_de_CEPs.csv

    thash h;
    int nbuckets = 1000;
    float taxa = 0.75;
    int tipo_hash = 1; // 0 = simples, 1 = dupla

    if (hash_constroi(&h, nbuckets, get_key, tipo_hash, taxa) != EXIT_SUCCESS) {
        printf("Erro ao construir tabela hash.\n");
        return EXIT_FAILURE;
    }

    if (carregar_ceps(arquivo_csv, &h) != EXIT_SUCCESS) {
        printf("Erro ao carregar dados do CSV.\n");
        return EXIT_FAILURE;
    }

    // Teste de busca
    const char *busca = "69945"; // Exemplo: CEP Inicial de Acrelândia
    treg *resultado = (treg *)hash_busca(&h, busca);
    if (resultado)
        printf("CEP: %s - Cidade: %s - Estado: %s\n", resultado->cep, resultado->cidade, resultado->estado);
    else
        printf("CEP %s não encontrado.\n", busca);


    //teste de tempo
    // carregar_ceps(tipo_hash, taxa,tamanho, qtd_registro);

    hash_apaga(&h);
    return 0;
}