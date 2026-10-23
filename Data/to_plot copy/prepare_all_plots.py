import pandas as pd
import os

# 1. Configurações
pasta_proc = os.path.join('..', 'processed')
pasta_plot = os.path.join('..', 'to_plot')

def preparar_csv(arq_st, col_st, arq_nt, col_nt, nome_saida):
    caminho_st = os.path.join(pasta_proc, arq_st)
    caminho_nt = os.path.join(pasta_proc, arq_nt)
    
    if os.path.exists(caminho_st) and os.path.exists(caminho_nt):
        # Carregar Estação
        df_st = pd.read_csv(caminho_st, sep=';')
        df_st['TIME'] = pd.to_datetime(df_st['TIME'], dayfirst=True)
        df_st = df_st[['TIME', col_st]].rename(columns={col_st: 'STATION_VAL'})

        # Carregar Rede
        df_nt = pd.read_csv(caminho_nt, sep=';')
        df_nt['TIME'] = pd.to_datetime(df_nt['TIME'])
        # Pega a coluna que contém o nome (geralmente é "Média" ou o nome do gás)
        df_nt = df_nt[['TIME', col_nt]].rename(columns={col_nt: 'NET_VAL'})

        # Merge Outer e Ordenação
        df_final = pd.merge(df_st, df_nt, on='TIME', how='outer').sort_values('TIME')
        
        caminho_saida = os.path.join(pasta_plot, nome_saida)
        df_final.to_csv(caminho_saida, index=False, sep=';')
        print(f"✅ Gerado: {nome_saida}")
    else:
        print(f"⚠️ Arquivo(s) não encontrado(s) para: {nome_saida}")

# 2. Executar para cada categoria
print("--- Gerando arquivos para plotagem ---")

# PTH
preparar_csv('station_temp.csv', 'TEMP_AVG', 'net_temp.csv', 'Temperatura Média', 'compare_temp.csv')
preparar_csv('station_hum.csv', 'HUM_AVG', 'net_hum.csv', 'Umidade Média', 'compare_hum.csv')
preparar_csv('station_press.csv', 'BME_PRESS', 'net_press.csv', 'Pressão Média', 'compare_press.csv')

# Anemômetro (Velocidade)
# preparar_csv('station_anemo.csv', 'ANEMO_VEL', 'net_anemovel.csv', 'Velocidade do Vento Média', 'compare_anemo.csv')

# Gases (Exemplo com H2S - Repita para outros se necessário)
preparar_csv('station_h2s.csv', 'H2S_PPB', 'net_h2s.csv', 'H2S PPB Médio', 'compare_h2s.csv')
preparar_csv('station_voc.csv', 'VOC_PPB', 'net_voc.csv', 'VOC PPB Médio', 'compare_voc.csv')
preparar_csv('station_so2.csv', 'SO2_PPB', 'net_so2.csv', 'SO2 PPB Médio', 'compare_so2.csv')
preparar_csv('station_nh3.csv', 'NH3_PPB', 'net_nh3.csv', 'NH3 PPB Médio', 'compare_nh3.csv')
preparar_csv('station_ch3sh.csv', 'CH3SH_PPB', 'net_ch3sh.csv', 'CH3SH PPB Médio', 'compare_ch3sh.csv')

print("\nTodos os datasets de comparação estão na pasta /to_plot/.")