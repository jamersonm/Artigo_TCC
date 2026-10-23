import pandas as pd
import numpy as np
import os
import random

# 1. Configurações de Caminho
pasta_to_plot = os.path.join('..', 'to_plot')

def calcular_metricas_com_falhas():
    arquivos = [f for f in os.listdir(pasta_to_plot) if f.startswith('compare_') and f.endswith('.csv')]
    
    if not arquivos:
        print("Nenhum arquivo encontrado em /to_plot.")
        return

    for arquivo in arquivos:
        caminho = os.path.join(pasta_to_plot, arquivo)
        df = pd.read_csv(caminho, sep=';')
        df['TIME'] = pd.to_datetime(df['TIME'])

        # --- SIMULAÇÃO DE FALHAS ---
        # 1. Identificar onde estão os pontos válidos
        idx_sim = df[df['SIMULATED_VAL'].notna()].index.tolist()
        idx_net = df[df['NET_VAL'].notna()].index.tolist()
        
        n_sim = len(idx_sim)
        n_net = len(idx_net)
        falhas_a_gerar = n_sim - n_net

        # Criamos uma cópia da coluna para não estragar o CSV original
        df['SIM_COM_FALHAS'] = df['SIMULATED_VAL'].copy()

        if falhas_a_gerar > 0:
            # Escolhe índices aleatórios entre os pontos da simulação para "apagar"
            indices_para_remover = random.sample(idx_sim, falhas_a_gerar)
            df.loc[indices_para_remover, 'SIM_COM_FALHAS'] = np.nan
            print(f"ℹ️ {arquivo}: Simulando {falhas_a_gerar} falhas na amostragem instantânea.")
        
        # --- CÁLCULO DE FIDELIDADE ---
        df_ref = df.dropna(subset=['STATION_VAL']).copy()

        # Estratégia A: Média (Rede) - Já possui as falhas reais
        df_ref['ESTRATEGIA_REDE'] = df['NET_VAL'].ffill()

        # Estratégia B: Instantânea (Simulada) - Agora com falhas artificiais
        df_ref['ESTRATEGIA_INST_FALHAS'] = df['SIM_COM_FALHAS'].ffill()

        # Limpar registros iniciais
        df_ref = df_ref.dropna(subset=['ESTRATEGIA_REDE', 'ESTRATEGIA_INST_FALHAS'])

        if df_ref.empty:
            continue

        # Cálculo de Erros
        df_ref['Erro_Rede'] = df_ref['ESTRATEGIA_REDE'] - df_ref['STATION_VAL']
        df_ref['Erro_Inst'] = df_ref['ESTRATEGIA_INST_FALHAS'] - df_ref['STATION_VAL']

        metrics = {
            "Métrica": ["MAE", "Erro Máximo", "RMSE"],
            "Aplicação (Rede - Média)": [
                df_ref['Erro_Rede'].abs().mean(),
                df_ref['Erro_Rede'].abs().max(),
                np.sqrt((df_ref['Erro_Rede']**2).mean())
            ],
            "Simulação (Inst - Com Falhas)": [
                df_ref['Erro_Inst'].abs().mean(),
                df_ref['Erro_Inst'].abs().max(),
                np.sqrt((df_ref['Erro_Inst']**2).mean())
            ]
        }

        print(f"\n" + "="*60)
        print(f"ANÁLISE DE ESTRESSE (Igualdade de Pacotes): {arquivo}")
        print(f"Pontos Comparados: {n_net} (Rede) vs {n_net} (Simulado c/ Falha)")
        print("="*60)
        print(pd.DataFrame(metrics).set_index("Métrica").round(4))

if __name__ == "__main__":
    # Definir seed para que o "aleatório" seja repetível se você rodar de novo
    random.seed(42)
    calcular_metricas_com_falhas()