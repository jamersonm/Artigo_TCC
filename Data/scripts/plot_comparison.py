import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Configuração de Caminhos
pasta_to_plot = os.path.join('..', 'to_plot')
pasta_img = os.path.join('..', 'img')

if not os.path.exists(pasta_img):
    os.makedirs(pasta_img)

# --- CONFIGURAÇÃO MANUAL DO PLOT ---
arquivo = 'compare_voc.csv' # Edite aqui o arquivo desejado

# Configuração de Intervalo (Ajustado para 1 semana conforme seu relato)
data_inicio = '2026-04-03 00:00:00'
data_fim    = '2026-04-03 23:59:59'

# 2. Carregamento dos dados
caminho_entrada = os.path.join(pasta_to_plot, arquivo)
if not os.path.exists(caminho_entrada):
    print(f"Erro: Arquivo {arquivo} não encontrado.")
else:
    df = pd.read_csv(caminho_entrada, sep=';')
    df['TIME'] = pd.to_datetime(df['TIME'])

    if data_inicio and data_fim:
        df = df[(df['TIME'] >= data_inicio) & (df['TIME'] <= data_fim)]

    # Preparar as séries
    df_1min = df.dropna(subset=['STATION_VAL'])
    df_sim  = df.dropna(subset=['SIMULATED_VAL'])
    df_net  = df.dropna(subset=['NET_VAL'])

    # --- CÁLCULO DE ESCALA UNIFICADA (Trava o Eixo Y para os 3 plots) ---
    y_min = df[['STATION_VAL', 'SIMULATED_VAL', 'NET_VAL']].min().min()
    y_max = df[['STATION_VAL', 'SIMULATED_VAL', 'NET_VAL']].max().max()
    margem = (y_max - y_min) * 0.05
    limite_y = (y_min - margem, y_max + margem)

    # 3. Criar a estrutura de 3 gráficos empilhados
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(16, 18), sharex=True)

    # SITUAÇÃO 1: Original (1 min) - O "Padrão Real"
    ax1.plot(df_1min['TIME'], df_1min['STATION_VAL'], color='skyblue', linewidth=1, label='Base 1min')
    ax1.set_title(f'SITUAÇÃO 1: registros locais com taxa de amostragem de 1 min.', fontsize=24, fontweight='bold')
    ax1.set_ylim(limite_y)
    ax1.grid(True, linestyle=':', alpha=0.6)

    # SITUAÇÃO 2: Simulado (30 min) - Sem perda de pacotes
    ax2.plot(df_sim['TIME'], df_sim['SIMULATED_VAL'], color='navy', linewidth=1.5, 
             marker='o', markersize=4, label='Simulado (Sem Furos)')
    ax2.set_title(f'SITUAÇÃO 2: registros locais com taxa de amostragem de 30 min.', fontsize=24, fontweight='bold')
    ax2.set_ylim(limite_y)
    ax2.grid(True, linestyle=':', alpha=0.6)

    # SITUAÇÃO 3: Rede (30 min) - Com perda de 41 pacotes
    ax3.plot(df_net['TIME'], df_net['NET_VAL'], color='crimson', linewidth=1.5, 
             marker='x', markersize=5, label='Rede (Com Furos)')
    ax3.set_title(f'SITUAÇÃO 3: registros recebidos na rede com taxa de amostragem de 30 min.', fontsize=24, fontweight='bold')
    ax3.set_ylim(limite_y)
    ax3.set_xlabel('Tempo (Data e Hora)', fontsize=12)
    ax3.grid(True, linestyle=':', alpha=0.6)

    # 4. Ajustes Finais
    plt.gcf().autofmt_xdate()
    plt.tight_layout(pad=4.0)

    nome_saida = f"comparativo_furos_{arquivo.replace('.csv', '')}.png"
    plt.savefig(os.path.join(pasta_img, nome_saida), dpi=300)
    
    print(f"✅ Plot gerado. Comparando {len(df_sim)} pontos (Simulado) vs {len(df_net)} pontos (Rede).")
    plt.show()