import pandas as pd
import os

# 1. Configuração de Caminhos
pasta_processed = os.path.join('..', 'processed')
pasta_to_plot = os.path.join('..', 'to_plot')

arq_estacao = os.path.join(pasta_processed, 'station_temp.csv')
arq_rede = os.path.join(pasta_processed, 'net_temp.csv')
arquivo_saida = os.path.join(pasta_to_plot, 'compare_temp.csv')

if not os.path.exists(pasta_to_plot):
    os.makedirs(pasta_to_plot)

# 2. Carregar Estação
df_st = pd.read_csv(arq_estacao, sep=';')
df_st['TIME'] = pd.to_datetime(df_st['TIME'], dayfirst=True)
df_st = df_st[['TIME', 'TEMP_AVG']].rename(columns={'TEMP_AVG': 'STATION_TEMP'})

# 3. Carregar Rede
df_nt = pd.read_csv(arq_rede, sep=';')
df_nt['TIME'] = pd.to_datetime(df_nt['TIME'])
df_nt = df_nt[['TIME', 'Temperatura Média']].rename(columns={'Temperatura Média': 'NET_TEMP'})

# 4. Merge Outer (União Total) e Ordenação Temporal
# O how='outer' garante que nenhuma linha de nenhum dos dois arquivos seja perdida
df_final = pd.merge(df_st, df_nt, on='TIME', how='outer')

# Ordenar por tempo para que a sequência faça sentido no gráfico
df_final = df_final.sort_values('TIME').reset_index(drop=True)

# 5. Salvar o arquivo para plotagem
df_final.to_csv(arquivo_saida, index=False, sep=';')

print(f"Dataset consolidado gerado com sucesso!")
print(f"Caminho: {arquivo_saida}")
print(f"Total de pontos temporais: {len(df_final)}")
print("\nExemplo do arquivo (Note os NaNs onde os tempos não coincidem):")
print(df_final.head(10))