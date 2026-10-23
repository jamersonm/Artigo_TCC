import pandas as pd
import os
from datetime import datetime

# 1. Caminhos
caminho_entrada = os.path.join('..', 'raw', 'raw_estacao.csv')
pasta_saida = os.path.join('..', 'processed')
arquivo_final = os.path.join(pasta_saida, 'station_time_fixed_and_filtered.csv')

# 2. Definição da Âncora Escolhida
tempo_sd_primeira = datetime(2026, 5, 25, 16, 0, 10) 
tempo_real_teste = datetime(2026, 3, 25, 16, 10, 0)
offset = tempo_sd_primeira - tempo_real_teste

# 3. Carregar e Corrigir
df = pd.read_csv(caminho_entrada, sep=';')
df['TIME'] = pd.to_datetime(df['TIME'], dayfirst=True)
df['TIME'] = df['TIME'] - offset

# 4. Filtro de Data Inicial (Início do Experimento)
data_inicio_experimento = datetime(2026, 3, 31, 11, 17, 0)
df_final = df[df['TIME'] >= data_inicio_experimento].copy()

# 5. Salvar na pasta raw_cropped
if not os.path.exists(pasta_saida):
    os.makedirs(pasta_saida)

df_final.to_csv(arquivo_final, index=False, sep=';', date_format='%d/%m/%y %H:%M:%S')

print(f"--- Processamento Finalizado ---")
print(f"Offset aplicado: {offset}")
print(f"Total de linhas salvas: {len(df_final)}")
print(f"Primeiro registro do experimento: {df_final['TIME'].iloc[0]}")
print(f"Arquivo pronto para os próximos passos: {arquivo_final}")