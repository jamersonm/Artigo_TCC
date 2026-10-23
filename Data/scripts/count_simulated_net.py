import pandas as pd
import os

# 1. Configurações de Caminho
pasta_to_plot = os.path.join('..', 'to_plot')

def auditar_todos_arquivos():
    # Listar todos os arquivos compare_ na pasta
    arquivos = [f for f in os.listdir(pasta_to_plot) if f.startswith('compare_') and f.endswith('.csv')]
    
    if not arquivos:
        print("Nenhum arquivo 'compare_' encontrado.")
        return

    # Lista para armazenar os resultados de cada arquivo
    resultados = []

    print(f"--- Iniciando Contagem de Amostras em {len(arquivos)} arquivos ---\n")

    for arquivo in arquivos:
        caminho = os.path.join(pasta_to_plot, arquivo)
        df = pd.read_csv(caminho, sep=';')
        
        # Realiza a contagem (ignora NaNs)
        count_1min = df['STATION_VAL'].count()
        count_sim  = df['SIMULATED_VAL'].count() if 'SIMULATED_VAL' in df.columns else 0
        count_net  = df['NET_VAL'].count() if 'NET_VAL' in df.columns else 0
        
        # Armazena em um dicionário para a tabela final
        resultados.append({
            'Arquivo': arquivo,
            'Leituras 1min': count_1min,
            'Amostras SIM (30m)': count_sim,
            'Amostras NET (30m)': count_net,
            'Diferença (S-N)': count_sim - count_net
        })

    # 2. Exibir em formato de Tabela
    df_resumo = pd.DataFrame(resultados)
    
    # Ordenar por nome do arquivo para facilitar leitura
    df_resumo = df_resumo.sort_values('Arquivo')
    
    print(df_resumo.to_string(index=False))
    
    # 3. Alerta de inconsistência
    inconsistentes = df_resumo[df_resumo['Diferença (S-N)'] != 0]
    if not inconsistentes.empty:
        print("\n⚠️  Alerta: Os arquivos acima com 'Diferença' diferente de zero possuem descasamento entre simulação e rede.")
    else:
        print("\n✅ Sucesso: Todas as amostragens de 30 min estão perfeitamente alinhadas.")

if __name__ == "__main__":
    auditar_todos_arquivos()