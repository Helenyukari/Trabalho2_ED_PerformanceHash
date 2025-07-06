import matplotlib.pyplot as plt

# Dados de exemplo - substitua pelos seus tempos reais do gprof
taxas_ocupacao = [10, 20, 30, 40, 50, 60, 70, 80, 90, 99]

# Tempo em milissegundos para cada taxa na hash simples
tempos_simples = [1.2, 1.5, 1.9, 2.4, 3.0, 3.8, 5.0, 6.8, 10.0, 15.0]

# Tempo em milissegundos para cada taxa na hash dupla
tempos_dupla = [1.5, 1.9, 2.5, 3.1, 4.0, 5.0, 6.5, 8.7, 13.0, 20.0]

plt.figure(figsize=(10, 6))

plt.plot(taxas_ocupacao, tempos_simples, marker='o', linestyle='-', color='blue', label='Hash Simples')
plt.plot(taxas_ocupacao, tempos_dupla, marker='s', linestyle='--', color='red', label='Hash Dupla')

plt.title('Tempo de Busca vs Taxa de Ocupação na Tabela Hash')
plt.xlabel('Taxa de Ocupação (%)')
plt.ylabel('Tempo de Busca (ms)')
plt.grid(True)
plt.legend()

plt.xticks(taxas_ocupacao)  # Marca os valores exatos no eixo X

plt.tight_layout()
plt.show()
