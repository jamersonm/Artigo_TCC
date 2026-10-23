import pandas as pd
import os

# 1. Configurações de Caminho
pasta_plot = os.path.join('..', 'to_plot')

# Listar todos os arquivos compare_
arquivos_compare = [f for f in os.listdir(pasta_plot) if f.startswith('compare_') and f.endswith('.csv')]

print(f"--- Adicionando SIMULATED_VAL (Instantâneo T+29) em {len(arquivos_compare)} arquivos ---")

for arquivo in arquivos_compare:
    caminho = os.path.join(pasta_plot, arquivo)
    
    # Carregar o CSV
    df = pd.read_csv(caminho, sep=';')
    
    # Garantir que a coluna SIMULATED_VAL comece vazia
    df['SIMULATED_VAL'] = pd.NA
    
    # 2. Lógica de Amostragem
    # Identificamos os índices onde a STATION_VAL não é nula
    indices_estacao = df[df['STATION_VAL'].notna()].index
    
    # Selecionamos a cada 30 registros, começando pelo 30º (índice 29 na lista de índices)
    # Isso pula as primeiras 29 leituras e pega a 30ª, 60ª, 90ª...
    indices_selecionados = indices_estacao[29::30]
    
    # 3. Copiar os valores
    # Atribuímos o valor da STATION_VAL para a SIMULATED_VAL apenas nesses índices
    df.loc[indices_selecionados, 'SIMULATED_VAL'] = df.loc[indices_selecionados, 'STATION_VAL']
    
    # Salvar o arquivo atualizado
    df.to_csv(caminho, index=False, sep=';')
    
    print(f"✅ {arquivo}: {len(indices_selecionados)} pontos de simulação adicionados.")

print("\nProcesso concluído. Agora seus arquivos possuem as 3 colunas para o plot final.")