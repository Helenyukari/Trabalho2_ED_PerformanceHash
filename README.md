###  Objetivo:  Fazer testes comparativos hash duplo vs hash simples em uma base de dados de CEPs
---
✅ 1 - Adapte o código dado em sala para que ele suporte hash simples e hash duplo.

✅ 2 - Adapte a estrutura da tabela hash para que armazene a taxa de ocupação da tabela. Altere a inserção para que ao atingir esta taxa de ocupação, o tamanho da tabela seja duplicada para suportar mais inserções. 

✅3 - Altere a estrutura para que dado os primeiros 5 digitos do CEP ele retorne o nome da cidade e o estado.

4 - Comparativos:

🔄 4.1 - Calcule o tempo de busca na Hash com taxa de ocupação de 10, 20, 30, 40, 50, 60, 70, 80, 90 e 99 % de ocupação.  Para este experimento considere uma tabela com 6100 buckets, faça funções específicas para cada taxa de ocupação (ex, busca10, busca20) e utilize gprof para medir o tempo de execução de cada função. Planilhe estes resultados e mostre um gráfico de taxa de ocupação (eixo x) versus tempo de execução (eixo y). Faça o comparativo de hash simples e dupla

 🔄4.2  - Faça a comparação de tempo de inserção de uma tabela com 6100 buckets iniciais  e com 1000 buckets iniciais e insira todas as cidades da base. Crie funções específicas para essa comparação (insere6100 e insere1000) e  com gprof apresente o tempo de execução e calcule o overhead de utilizar uma estrutura dinâmica de inserção. O objetivo aqui é verificar o tempo tomado para crescer a estrutura.

---


### ❔ Como executar  👩‍💻:

para executar o codigo hash.c


compile nomeando o ./exe:

    gcc -o codigo .\hash.c 


execute enviando uma argumento, nesse caso a base de dados em csv:

    .\codigo.exe .\Lista_de_CEPs.csv

 🐙🐙🐙como fiz pra inicializar um repositorio e guardar os arquivos🐙🐙🐙 (OBS: só pra mim lembrar)

    git init

    git add .

    commit -m "Primeiro commit: configuracao inicial do projeto"

    git remote add origin https://github.com/Helenyukari/Trabalho2_ED_PerformanceHash.git

    branch -M main
    
    git push -u origin main