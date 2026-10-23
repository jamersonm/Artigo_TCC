import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Configuração de Caminhos
pasta_to_plot = os.path.join('..', 'to_plot')
pasta_img = os.path.join('..', 'img')

if not os.path.exists(pasta_img):
    os.makedirs(pasta_img)

# --- CONFIGURAÇÃO DE INTERVALO GLOBAL ---
# Defina o período que deseja que apareça em todos os gráficos
data_inicio = '2026-03-31 11:17:00'
data_fim    = '2026-04-07 12:00:00'
# data_inicio, data_fim = None, None # Descomente para usar o arquivo inteiro

def gerar_todos_os_plots():
    # Listar todos os arquivos de comparação
    arquivos = [f for f in os.listdir(pasta_to_plot) if f.startswith('compare_') and f.endswith('.csv')]
    
    if not arquivos:
        print("Nenhum arquivo 'compare_' encontrado em /to_plot.")
        return

    print(f"--- Iniciando Geração em Massa: {len(arquivos)} sensores ---")

    for arquivo in arquivos:
        try:
            caminho_entrada = os.path.join(pasta_to_plot, arquivo)
            df = pd.read_csv(caminho_entrada, sep=';')
            df['TIME'] = pd.to_datetime(df['TIME'])

            # Filtro de Intervalo
            if data_inicio and data_fim:
                df = df[(df['TIME'] >= data_inicio) & (df['TIME'] <= data_fim)]

            if df.empty:
                print(f"⚠️ {arquivo}: Sem dados no intervalo selecionado.")
                continue

            # Preparar as séries (remover NaNs para linhas contínuas)
            df_1min = df.dropna(subset=['STATION_VAL'])
            df_sim  = df.dropna(subset=['SIMULATED_VAL'])
            df_net  = df.dropna(subset=['NET_VAL'])

            # --- CÁLCULO DE ESCALA UNIFICADA ---
            y_min = df[['STATION_VAL', 'SIMULATED_VAL', 'NET_VAL']].min().min()
            y_max = df[['STATION_VAL', 'SIMULATED_VAL', 'NET_VAL']].max().max()
            margem = (y_max - y_min) * 0.05
            limite_y = (y_min - margem, y_max + margem)

            # 3. Criar a estrutura de 3 gráficos empilhados
            fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(16, 18), sharex=True)

            # SITUAÇÃO 1: Referência Local (1 min)
            ax1.plot(df_1min['TIME'], df_1min['STATION_VAL'], color='skyblue', linewidth=1)
            ax1.set_title(f'SITUAÇÃO 1: Referência Local Bruta (1 min) - {arquivo}', fontsize=14, fontweight='bold')
            ax1.set_ylim(limite_y)
            ax1.grid(True, linestyle=':', alpha=0.6)

            # SITUAÇÃO 2: Média Real Calculada (Gabarito)
            # Agora plotamos como linha + marcador para mostrar a média perfeita de 30min
            ax2.plot(df_sim['TIME'], df_sim['SIMULATED_VAL'], color='navy', linewidth=1.5, 
                     marker='o', markersize=4, label='Média Real (Python)')
            ax2.set_title(f'SITUAÇÃO 2: Média Real do Período (Gabarito 30 min)', fontsize=14, fontweight='bold')
            ax2.set_ylim(limite_y)
            ax2.grid(True, linestyle=':', alpha=0.6)

            # SITUAÇÃO 3: Média Recebida pela Rede (Aplicação)
            ax3.plot(df_net['TIME'], df_net['NET_VAL'], color='crimson', linewidth=1.5, 
                     marker='x', markersize=5, label='Média Rede (Firmware)')
            ax3.set_title(f'SITUAÇÃO 3: Dados Recebidos na Rede (Média Processada)', fontsize=14, fontweight='bold')
            ax3.set_ylim(limite_y)
            ax3.set_xlabel('Tempo', fontsize=12)
            ax3.grid(True, linestyle=':', alpha=0.6)

            # 4. Ajustes Finais e Salvamento
            plt.gcf().autofmt_xdate()
            plt.tight_layout(pad=4.0)

            nome_saida = f"relatorio_triplo_{arquivo.replace('.csv', '')}.png"
            plt.savefig(os.path.join(pasta_img, nome_saida), dpi=300)
            plt.close() # Importante para liberar memória no loop
            
            print(f"✅ Gerado: {nome_saida}")

        except Exception as e:
            print(f"❌ Erro ao processar {arquivo}: {e}")

    print("\nProcesso de plotagem concluído!")

if __name__ == "__main__":
    gerar_todos_os_plots()