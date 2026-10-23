import pandas as pd
import numpy as np
import os

# 1. Configurações de Caminho
pasta_to_plot = os.path.join('..', 'to_plot')

def calcular_mae_justo():
    arquivos = [f for f in os.listdir(pasta_to_plot) if f.startswith('compare_') and f.endswith('.csv')]
    
    for arquivo in arquivos:
        caminho = os.path.join(pasta_to_plot, arquivo)
        df_raw = pd.read_csv(caminho, sep=';')
        
        # Converter tempo e ordenar (obrigatório para merge_asof)
        df_raw['TIME'] = pd.to_datetime(df_raw['TIME'])
        df_raw = df_raw.sort_values('TIME')

        # --- SEPARAÇÃO DOS DADOS ---
        # 1. A "Verdade" (Estação 1 min)
        df_st = df_raw.dropna(subset=['STATION_VAL']).copy()
        
        # 2. As Estratégias (Rede e Simulado)
        df_net = df_raw.dropna(subset=['NET_VAL']).copy()
        df_sim = df_raw.dropna(subset=['SIMULATED_VAL']).copy()

        # --- O JUIZ: Média Real de 1min ---
        # Calculamos a média real dos últimos 30min para cada ponto da rede
        dados_comp = []

        # Iteramos sobre os pontos que chegaram na REDE
        for _, row in df_net.iterrows():
            hora_atual = row['TIME']
            hora_inicio = hora_atual - pd.Timedelta(minutes=30)

            # Busca na estação os 30 min anteriores a esse envio da rede
            janela = df_st[(df_st['TIME'] > hora_inicio) & (df_st['TIME'] <= hora_atual)]
            
            if len(janela) >= 15: # Exige pelo menos metade dos dados (15 min) para ser justo
                media_real = janela['STATION_VAL'].mean()
                
                # Busca o SIMULATED_VAL mais próximo desse timestamp da rede (tolerância 5 min)
                # Isso resolve o problema de segundos/minutos desalinhados no CSV
                diff_sim = (df_sim['TIME'] - hora_atual).abs()
                idx_sim_prox = diff_sim.idxmin()
                
                if diff_sim.min() < pd.Timedelta(minutes=5):
                    val_sim = df_sim.loc[idx_sim_prox, 'SIMULATED_VAL']
                    
                    dados_comp.append({
                        'TIME': hora_atual,
                        'VERDADE_REAL': media_real,
                        'NET_VAL': row['NET_VAL'],
                        'SIMULATED_VAL': val_sim
                    })

        df_final = pd.DataFrame(dados_comp)

        if df_final.empty:
            print(f"⚠️ {arquivo}: Sem dados sobrepostos entre Estação, Simulado e Rede.")
            continue

        # --- 3. CÁLCULO DO ERRO ---
        df_final['Erro_Rede'] = df_final['NET_VAL'] - df_final['VERDADE_REAL']
        df_final['Erro_Inst'] = df_final['SIMULATED_VAL'] - df_final['VERDADE_REAL']

        metrics = {
            "Métrica": ["MAE (Fidelidade)", "Erro Máximo", "RMSE"],
            "Aplicação (Média Rede)": [
                df_final['Erro_Rede'].abs().mean(),
                df_final['Erro_Rede'].abs().max(),
                np.sqrt((df_final['Erro_Rede']**2).mean())
            ],
            "Simulação (Ponto Inst)": [
                df_final['Erro_Inst'].abs().mean(),
                df_final['Erro_Inst'].abs().max(),
                np.sqrt((df_final['Erro_Inst']**2).mean())
            ]
        }

        print(f"\n" + "="*65)
        print(f"VALIDAÇÃO JUSTA: {arquivo} ({len(df_final)} ciclos analisados)")
        print("="*65)
        resumo = pd.DataFrame(metrics).set_index("Métrica")
        print(resumo.round(4))

        mae_rede = resumo.loc["MAE (Fidelidade)", "Aplicação (Média Rede)"]
        mae_inst = resumo.loc["MAE (Fidelidade)", "Simulação (Ponto Inst)"]
        
        if mae_rede < mae_inst:
            print(f"✅ Veredito: MÉDIA ganha (mais representativa).")
        else:
            print(f"❌ Veredito: INSTANTÂNEA ganha (média com erro de cálculo).")

if __name__ == "__main__":
    calcular_mae_justo()