import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# 1. Configuração de Caminhos
pasta_to_plot = os.path.join('..', 'to_plot')
pasta_img = os.path.join('..', 'img')

if not os.path.exists(pasta_img):
    os.makedirs(pasta_img)

# --- CONFIGURAÇÃO MANUAL DO PLOT ---
arquivo = 'compare_h2s.csv'
data_inicio = '2026-03-31 11:17:00'
data_fim    = '2026-03-31 14:00:00'

# 2. Carregamento dos dados
caminho_entrada = os.path.join(pasta_to_plot, arquivo)
if not os.path.exists(caminho_entrada):
    print(f"Erro: Arquivo {arquivo} não encontrado.")
else:
    df = pd.read_csv(caminho_entrada, sep=';')
    df['TIME'] = pd.to_datetime(df['TIME'])
    df = df[(df['TIME'] >= data_inicio) & (df['TIME'] <= data_fim)].sort_values('TIME')

    # NOVA FUNÇÃO: Cálculo de Área por Soma de Riemann (Retângulos)
    # Área = Soma(Valor * Delta_T_em_horas)
    def calcular_area_riemann(df_serie, col_val, intervalo_minutos):
        if df_serie.empty: return 0
        delta_t_horas = intervalo_minutos / 60
        # Multiplicamos cada registro pelo tempo que ele representa
        return (df_serie[col_val] * delta_t_horas).sum()

    # Preparar as séries
    df_ref  = df.dropna(subset=['STATION_VAL'])    # S1: Referência
    df_sub  = df.dropna(subset=['SIMULATED_VAL'])  # S2: Subamostragem (Ref a cada 30min)
    df_met  = df.dropna(subset=['NET_VAL'])        # S3: Métricas (Média de 30 registros)

    # Cálculo das Áreas (Ponderadas pelo intervalo de amostragem)
    area_1 = calcular_area_riemann(df_ref, 'STATION_VAL', 1)   # Cada ponto representa 1 min
    area_2 = calcular_area_riemann(df_sub, 'SIMULATED_VAL', 30) # Cada ponto representa 30 min
    area_3 = calcular_area_riemann(df_met, 'NET_VAL', 30)       # Cada ponto representa 30 min

    # Cálculo de Erro relativo (%)
    erro_2 = (abs(area_2 - area_1) / area_1) * 100 if area_1 > 0 else 0
    erro_3 = (abs(area_3 - area_1) / area_1) * 100 if area_1 > 0 else 0

    # 3. Plotagem
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(16, 18), sharex=True)

    y_max = df[['STATION_VAL', 'SIMULATED_VAL', 'NET_VAL']].max().max() * 1.2

    # SITUAÇÃO 1: Referência
    ax1.bar(df_ref['TIME'], df_ref['STATION_VAL'], width=0.0006, color='skyblue', alpha=0.5, label='Ref 1min')
    ax1.set_title(f'SITUAÇÃO 1: Referência (1 min) | ÁREA: {area_1:.2f} ppb.h', fontsize=14, fontweight='bold')

    # SITUAÇÃO 2: Subamostra (Instantâneo a cada 30min)
    # Usamos drawstyle='steps-post' para visualizar a área retangular que o cálculo assume
    ax2.step(df_sub['TIME'], df_sub['SIMULATED_VAL'], where='post', color='navy', linewidth=1.5, label='Subamostra 30min')
    ax2.fill_between(df_sub['TIME'], df_sub['SIMULATED_VAL'], step="post", color='navy', alpha=0.2)
    ax2.set_title(f'SITUAÇÃO 2: Ref. Subamostrada (30 min) | ÁREA: {area_2:.2f} ppb.h (Erro: {erro_2:.2f}%)', fontsize=14, fontweight='bold')

    # SITUAÇÃO 3: Métricas (Média 30min)
    ax3.step(df_met['TIME'], df_met['NET_VAL'], where='post', color='crimson', linewidth=1.5, label='Métricas 30min')
    ax3.fill_between(df_met['TIME'], df_met['NET_VAL'], step="post", color='crimson', alpha=0.2)
    ax3.set_title(f'SITUAÇÃO 3: Cálculo de Métricas (30 min) | ÁREA: {area_3:.2f} ppb.h (Erro: {erro_3:.2f}%)', fontsize=14, fontweight='bold')

    for ax in [ax1, ax2, ax3]:
        ax.set_ylim(0, y_max)
        ax.set_ylabel('H2S (ppb)')
        ax.grid(True, linestyle=':', alpha=0.6)
        ax.legend(loc='upper right')

    ax3.set_xlabel('Tempo (Data e Hora)', fontsize=12)

    plt.gcf().autofmt_xdate()
    plt.tight_layout(pad=4.0)

    # Exibir resultados
    print(f"\n--- ANÁLISE DE CARGA (SOMA DE RIEMANN) ---")
    print(f"Área Referência (S1): {area_1:.4f} ppb.h")
    print(f"Área Subamostra (S2):  {area_2:.4f} ppb.h (Erro: {erro_2:.2f}%)")
    print(f"Área Métricas (S3):    {area_3:.4f} ppb.h (Erro: {erro_3:.2f}%)")
    
    plt.show()