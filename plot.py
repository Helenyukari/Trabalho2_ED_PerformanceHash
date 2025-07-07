import matplotlib.pyplot as plt

# Dados extraídos do relatório da Helen
taxas_ocupacao = [10, 20, 30, 40, 50, 60, 70, 80, 90, 99]

# Tempos de inserção (em segundos)
hash_simples_insercao = [0.007195, 0.005942, 0.004763, 0.006411, 0.004934, 0.006188, 0.006040, 0.005985, 0.007515, 0.011644]
hash_duplo_insercao = [0.004293, 0.006680, 0.005604, 0.004852, 0.009073, 0.009614, 0.005702, 0.006394, 0.013656, 0.006172]

# Tempos de busca (em segundos)
hash_simples_busca = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.021981]
hash_duplo_busca = [0.0, 0.0, 0.001095, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

# Plotagem
plt.figure(figsize=(12, 6))

# Inserção
plt.subplot(1, 2, 1)
plt.plot(taxas_ocupacao, hash_simples_insercao, marker='o', label='Hash Simples')
plt.plot(taxas_ocupacao, hash_duplo_insercao, marker='s', label='Hash Duplo')
plt.title('Tempo de Inserção por Taxa de Ocupação')
plt.xlabel('Taxa de Ocupação (%)')
plt.ylabel('Tempo (s)')
plt.grid(True)
plt.legend()

# Busca
plt.subplot(1, 2, 2)
plt.plot(taxas_ocupacao, hash_simples_busca, marker='o', label='Hash Simples')
plt.plot(taxas_ocupacao, hash_duplo_busca, marker='s', label='Hash Duplo')
plt.title('Tempo de Busca por Taxa de Ocupação')
plt.xlabel('Taxa de Ocupação (%)')
plt.ylabel('Tempo (s)')
plt.grid(True)
plt.legend()

plt.tight_layout()
plt.show()
