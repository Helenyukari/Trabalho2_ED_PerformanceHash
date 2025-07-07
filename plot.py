import matplotlib.pyplot as plt

# Taxas de ocupação testadas
taxas_ocupacao = [10, 20, 30, 40, 50, 60, 70, 80, 90, 99]

# Tempos extraídos do gprof (ms/call - total) para cada buscaXX
tempos_hash_simples = [2.63] * len(taxas_ocupacao)

plt.figure(figsize=(10, 6))
plt.plot(taxas_ocupacao, tempos_hash_simples, marker='o', linestyle='-', color='blue', label='Hash Simples (dados reais)')

plt.title('Tempo de Busca vs Taxa de Ocupação - Hash Simples')
plt.xlabel('Taxa de Ocupação (%)')
plt.ylabel('Tempo de Busca (ms)')
plt.grid(True)
plt.legend()
plt.xticks(taxas_ocupacao)

plt.tight_layout()
plt.show()
