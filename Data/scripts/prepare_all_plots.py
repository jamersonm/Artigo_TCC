import pandas as pd
import os

# 1. Configurações
pasta_proc = os.path.join('..', 'processed')
pasta_plot = os.path.join('..', 'to_plot')

if not os.path.exists(pasta_plot):
    os.makedirs(pasta_plot)

# --- CONFIGURAÇÃO DE INTERVALO ---
# Defina aqui o período exato para o recorte
data_inicio = '2026-03-31 11:17:00'
data_fim    = '2026-04-07 12:00:00'

def preparar_csv(arq_st, col_st, arq_nt, col_nt, nome_saida):
    caminho_st = os.path.join(pasta_proc, arq_st)
    caminho_nt = os.path.join(pasta_proc, arq_nt)
    
    if os.path.exists(caminho_st) and os.path.exists(caminho_nt):
        # --- 1. ESTAÇÃO ---
        df_st = pd.read_csv(caminho_st, sep=';')
        df_st['TIME'] = pd.to_datetime(df_st['TIME'], dayfirst=True)
        
        # Filtro de Intervalo para Estação
        df_st = df_st[(df_st['TIME'] >= data_inicio) & (df_st['TIME'] <= data_fim)]
        df_st = df_st[['TIME', col_st]].rename(columns={col_st: 'STATION_VAL'})

        # --- 2. REDE ---
        df_nt = pd.read_csv(caminho_nt, sep=';')
        df_nt['TIME'] = pd.to_datetime(df_nt['TIME'])
        
        # Filtro de Intervalo para Rede
        df_nt = df_nt[(df_nt['TIME'] >= data_inicio) & (df_nt['TIME'] <= data_fim)]
        df_nt = df_nt[['TIME', col_nt]].rename(columns={col_nt: 'NET_VAL'})

        # --- 3. MERGE E SALVAMENTO ---
        # Merge Outer para manter os carimbos de tempo de ambos
        df_final = pd.merge(df_st, df_nt, on='TIME', how='outer').sort_values('TIME')
        
        caminho_saida = os.path.join(pasta_plot, nome_saida)
        df_final.to_csv(caminho_saida, index=False, sep=';')
        print(f"✅ Gerado: {nome_saida} | Intervalo aplicado.")
    else:
        print(f"⚠️ Arquivo(s) não encontrado(s) para: {nome_saida}")

# 2. Executar para cada categoria
print(f"--- Iniciando Preparação (Filtro: {data_inicio} a {data_fim}) ---")

# PTH
preparar_csv('station_temp.csv', 'TEMP_AVG', 'net_temp.csv', 'Temperatura Média', 'compare_temp.csv')
preparar_csv('station_hum.csv', 'HUM_AVG', 'net_hum.csv', 'Umidade Média', 'compare_hum.csv')
preparar_csv('station_press.csv', 'BME_PRESS', 'net_press.csv', 'Pressão Média', 'compare_press.csv')

# Gases
gases = [
    ('station_h2s.csv', 'H2S_PPB', 'net_h2s.csv', 'H2S PPB Médio', 'compare_h2s.csv'),
    ('station_voc.csv', 'VOC_PPB', 'net_voc.csv', 'VOC PPB Médio', 'compare_voc.csv'),
    ('station_so2.csv', 'SO2_PPB', 'net_so2.csv', 'SO2 PPB Médio', 'compare_so2.csv'),
    ('station_nh3.csv', 'NH3_PPB', 'net_nh3.csv', 'NH3 PPB Médio', 'compare_nh3.csv'),
    ('station_ch3sh.csv', 'CH3SH_PPB', 'net_ch3sh.csv', 'CH3SH PPB Médio', 'compare_ch3sh.csv')
]

for g in gases:
    preparar_csv(*g)

print("\nProcesso concluído. Arquivos salvos em /to_plot/")